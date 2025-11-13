using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Threading.Tasks;
using TestFw;
using testfw_app.Helper;
using Windows.Foundation;
using Windows.Graphics.Display;
using Windows.Storage;
using Windows.System.Threading;
using Windows.UI.Popups;
using Windows.UI.Xaml;
using WindowsPhoneInterop;

namespace testfw_app.Model
{
    #region Delegates
    public delegate void TestInitializedDelegate(bool successed, string result);
    public delegate void TestInitializProgressChangeDelegate(double progress);
    public delegate void OnVoidDelegate();
    public delegate void ConnectionError(string message);
    #endregion

    public enum TestStatus
    {
        NONE,
        CANCELLED,
        SKIPPED,
        LOADING,
        RUNNING,
        FINISHED
    }

    public class PlannedTest
    {
        #region Events
        public event TestInitializedDelegate OnTestInitialized;
        public event TestInitializProgressChangeDelegate OnProgressChanged;
        public event OnVoidDelegate OnSkipped;
        public event OnVoidDelegate OnTestFinished;
        public event OnVoidDelegate OnCancelled;
        public event ConnectionError OnConnectionError;
        #endregion

        #region Attributes
        private double prevProgress;
        private TestBase currentTest;
        private IAsyncAction workThread;
        private IAsyncAction updateThread;

#if BATTERY_TEST
        private TfwMessageQueue msgq = new TfwMessageQueue();
        private ThreadPoolTimer _periodicTimer;
#endif

        public string Result { get; set; }
        public TestData TestData { get; set; }
        public TestStatus Status { get; set; }

        public Size NativeResolution { get; set; }
        public bool ValidScreenResolution { get; set; }

        private int dummy = 100;
        private bool isBattery;
        #endregion

        #region BatteryTest
        internal bool IsBatteryTest()
        {
            return isBattery;
        }

#if BATTERY_TEST
        private void StartTimer(int ms)
        {
            _periodicTimer = ThreadPoolTimer.CreatePeriodicTimer(UpdateBatteryHandler, TimeSpan.FromMilliseconds(ms));

            if (Parameters.IsDummyTest)
            {
                Message msg = new Message(0xBA7, SystemInfo.Battery.Instance.IsCharging ? 1 : 0, SystemInfo.Battery.Instance.BateryLevel, 0);
                msgq.push_back(msg);
            }
            else
            {
                Message msg = new Message(0xBA7, 0, 100, 0);
                msgq.push_back(msg);
            }
        }

        private void StopTimer()
        {
            if (_periodicTimer != null)
                _periodicTimer.Cancel();
        }

        private void UpdateBatteryHandler(ThreadPoolTimer timer)
        {
            Message msg = null;
            if (Parameters.IsDummyTest)
            {
                dummy -= 5;
                msg = new Message(0xBA7, 0, dummy, 0);
            }
            else
            {
                msg = new Message(0xBA7, SystemInfo.Battery.Instance.IsCharging ? 1 : 0, SystemInfo.Battery.Instance.BateryLevel, 0);
            }
            msgq.push_back(msg);
        }
#endif
        #endregion

        public PlannedTest(TestData data)
        {
            Status = TestStatus.NONE;
            TestData = data;

            Result = null;
            currentTest = null;
            ValidScreenResolution = false;
        }

        public async Task Init(ContextFactory contextFactory)
        {
            Status = TestStatus.LOADING;
            prevProgress = -1;

            bool successed = false;

            JObject raw_config = TestData.Desc["raw_config"] as JObject;
            bool offscreen = (int)raw_config["screenmode"] == 1;
            bool virtual_resolution = false;
            isBattery = (bool)raw_config["battery"];

            Size resolution;
            if (Parameters.Width != 0 && Parameters.Height != 0)
            {
                virtual_resolution = true;
                // Set custom resolution in pixels
                resolution.Width = Parameters.Width;
                resolution.Height = Parameters.Height;
            }
            else
            {
                virtual_resolution = false;
                // Set native resolution in pixels
                resolution.Width = (Int32)NativeResolution.Width;
                resolution.Height = (Int32)NativeResolution.Height;
            }

            string folder = IOHelper.GetLocalFolder().Path;

            JObject env = TestData.Desc.GetValue("env") as JObject;
            env["read_path"] = String.Format("{0}\\{1}", folder, TestData.Desc["data_prefix"]);
            env["write_path"] = String.Format("{0}\\{1}", folder, TestData.Desc["data_prefix"]);

            Debug.WriteLine("offscreen: {0} native_res: {1} custom_res: {2}", offscreen, NativeResolution, resolution);

            if (offscreen)
            {
                if (virtual_resolution)
                {
                    raw_config["test_width"] = resolution.Width;
                    raw_config["test_height"] = resolution.Height;
                }

                env["width"] = NativeResolution.Width;
                env["height"] = NativeResolution.Height;
            }
            else
            {
                env["width"] = resolution.Width;
                env["height"] = resolution.Height;
            }
            
            foreach (KeyValuePair<string,double> item in Parameters.rawn)
            {
                raw_config[item.Key] = item.Value;
            }
            foreach (KeyValuePair<string, bool> item in Parameters.rawz)
            {
                raw_config[item.Key] = item.Value;
            }
            foreach (KeyValuePair<string, string> item in Parameters.raws)
            {
                raw_config[item.Key] = item.Value;
            }
                        
            String factoryMethod = TestData.Desc.GetValue("factory_method").ToString();
            TestData.Desc["env"] = env;

            ///////////DEBUG/////////////
            if (Parameters.IsDummyTest)
            {
                raw_config["play_time"] = 100;
                //raw_config["frame_step_time"] = 1;
            }
            /////////////////////////////

            String finalDesc = TestData.Desc.ToString();

            TestFactory factory = TestFactory.test_factory(factoryMethod);
            if (factory.valid())
            {
                currentTest = factory.create_test();
                if (currentTest != null)
                {
                    currentTest.setConfig(finalDesc);

                    long i_ptr = contextFactory.getContext();
                    IntPtr ptr = new IntPtr(i_ptr);

                    GraphicsContext graphicsCtx = new GraphicsContext(ptr, false);
                    currentTest.setGraphicsContext(graphicsCtx);

                    JObject g_cfg = (JObject)((JObject)env["graphics"])["config"];
                    contextFactory.setFormat((int)g_cfg["red_size"], (int)g_cfg["green_size"], (int)g_cfg["blue_size"], (int)g_cfg["depth_size"], (int)g_cfg["stencil_size"], (int)g_cfg["samples"]);

                    if (offscreen)
                    {
                        ValidScreenResolution = contextFactory.SetResolution(NativeResolution);
                    }
                    else
                    {
                        ValidScreenResolution = contextFactory.SetResolution(resolution);
                    }
                }

#if BATTERY_TEST
                if (isBattery)
                {
                    currentTest.setMessageQueue(msgq);
                    StartTimer(10000);
                }
#endif

                // Create a task that will be run on a background thread.
                var workItemHandler = new WorkItemHandler(async (IAsyncAction action) =>
                {
                    Debug.WriteLine("init");
                    successed = currentTest.init();

                    if (OnTestInitialized != null)
                    {
                        OnTestInitialized(successed, currentTest.result());
                    }

                    if (successed)
                    {
                        Debug.WriteLine("run");
                        Status = TestStatus.RUNNING;
                        currentTest.run();
                    }
                    else
                    {
                        Status = TestStatus.SKIPPED;
                        if (OnSkipped != null)
                            OnSkipped();
                    }

#if BATTERY_TEST
                    if (isBattery)
                    {
                        StopTimer();
                    }
#endif

                    Result = currentTest.result();

                    JObject jsonValue = JObject.Parse(Result);
                    JArray array = jsonValue["results"] as JArray;
                    if (array.Count > 0)
                    {
                        JObject subresult = array[0] as JObject;
                        Status = subresult["status"].Value<String>() == "CANCELLED" ? TestStatus.CANCELLED : Status;
                    }
                    Debug.WriteLine(Result);

                    if (Status != TestStatus.CANCELLED)
                        Status = TestStatus.FINISHED;

                    if (Status == TestStatus.FINISHED)
                    {
                        Debug.WriteLine("finished");

                        if (OnTestFinished != null)
                            OnTestFinished();
                    }
                    else if (Status == TestStatus.CANCELLED)
                    {
                        Debug.WriteLine("cancelled");

                        if (OnCancelled != null)
                            OnCancelled();
                    }
                });

                // update current test progess bar value
                var updateItemHandler = new WorkItemHandler((IAsyncAction action) =>
                {
                    Debug.WriteLine("update thread");
                    while (Status == TestStatus.LOADING)
                    {
                        if (OnProgressChanged != null)
                        {
                            float progress = currentTest.progress();

                            if (prevProgress != progress)
                            {
                                prevProgress = progress;
                                OnProgressChanged(progress);
                            }
                        }
                    }
                });

                if (currentTest != null && ValidScreenResolution)
                {
                    workThread = ThreadPool.RunAsync(workItemHandler, WorkItemPriority.Low, WorkItemOptions.TimeSliced);
                    updateThread = ThreadPool.RunAsync(updateItemHandler, WorkItemPriority.Normal, WorkItemOptions.TimeSliced);
                }
                else
                {
                    Status = TestStatus.SKIPPED;
                    if (OnSkipped != null)
                        OnSkipped();
                }
            }
            else
            {
                if (OnSkipped != null)
                    OnSkipped();
            }
        }

        public void Cancel()
        {
            Status = TestStatus.CANCELLED;

            if (updateThread != null && updateThread.Status == AsyncStatus.Started)
                updateThread.Cancel();

            if (currentTest != null)
            {
                currentTest.cancel();
            }
        }
    }
}
