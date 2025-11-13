using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Graphics.Display;
using Windows.System.Threading;
using Windows.UI.Core;
using Windows.UI.Popups;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using System.Runtime.CompilerServices;
using Windows.Storage;
using System.IO;
using Windows.Storage.Streams;
using WindowsPhoneInterop;
using WindowsPhoneApp;

namespace testfw_app.Model
{
    public class TestPageModel : INotifyPropertyChanged
    {
        #region Events
        public event PropertyChangedEventHandler PropertyChanged;
        #endregion

        #region Bindable properties
        private bool _loading;
        public bool IsLoading
        {
            get { return _loading; }
            set { _loading = value; OnPropertyChanged("IsLoading"); }
        }

        private double _progress;
        public double ProgressValue
        {
            get { return _progress; }
            set { _progress = value; OnPropertyChanged("ProgressValue"); }
        }

        private String _loadingTestId;
        public String LoadingTestId
        {
            get { return _loadingTestId; }
            set { _loadingTestId = value; OnPropertyChanged("LoadingTestId"); }
        }

        private String _loadingTestName;
        public String LoadingTestName
        {
            get { return _loadingTestName; }
            set { _loadingTestName = value; OnPropertyChanged("LoadingTestName"); }
        }

        private String _loadingDesc;
        public String LoadingDesc
        {
            get { return _loadingDesc; }
            set { _loadingDesc = value; OnPropertyChanged("LoadingDesc"); }
        }

        private String _loadingString;
        public String LoadingString
        {
            get { return _loadingString; }
            set { _loadingString = value; OnPropertyChanged("LoadingString"); }
        }

        // Create the OnPropertyChanged method to raise the event 
        protected void OnPropertyChanged(string replace = null, [CallerMemberName] string name = "")
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                if (replace != null)
                    handler(this, new PropertyChangedEventArgs(replace));
                else
                    handler(this, new PropertyChangedEventArgs(name));
            }
        }
        #endregion

        #region Attributes
        private CoreWindow window = Window.Current.CoreWindow;
        private CoreDispatcher Dispatcher;
        public Frame Frame { get; set; }

        private ContextFactory contextFactory;
        private List<PlannedTest> tests = new List<PlannedTest>();
        private int actualTest;
        private StorageFolder _resultFolder;
        #endregion

        public TestPageModel(CoreDispatcher dispatcher, SwapChainPanel swapChainPanel)
        {
            this.Dispatcher = dispatcher;
            Window.Current.Activated += Current_Activated;
            Window.Current.Closed += Current_Closed;
            Window.Current.SizeChanged += Current_SizeChanged;
            Window.Current.VisibilityChanged += Current_VisibilityChanged;
            Window.Current.Content.LostFocus += Content_LostFocus;
            window.VisibilityChanged += window_VisibilityChanged;
            window.SizeChanged += window_SizeChanged;
            window.PointerCaptureLost += window_PointerCaptureLost;

            DisplayInformation.DisplayContentsInvalidated += DisplayInformation_DisplayContentsInvalidated;

            contextFactory = new ContextFactory();
            contextFactory.SetSwapChainPanel(swapChainPanel);
            App.Current.Suspending += Current_Suspending;

            Window.Current.CoreWindow.SetPointerCapture();
            Window.Current.CoreWindow.PointerExited += CoreWindow_PointerExited;
            Window.Current.CoreWindow.PointerCaptureLost += CoreWindow_PointerCaptureLost;
        }

        #region Cancel Events
        void CoreWindow_PointerExited(CoreWindow sender, PointerEventArgs args)
        {
        }

        void CoreWindow_PointerCaptureLost(CoreWindow sender, PointerEventArgs args)
        {
        }

        void Content_LostFocus(object sender, RoutedEventArgs e)
        {
        }

        void Current_VisibilityChanged(object sender, VisibilityChangedEventArgs e)
        {
        }

        void Current_SizeChanged(object sender, WindowSizeChangedEventArgs e)
        {
        }

        void Current_Closed(object sender, CoreWindowEventArgs e)
        {
        }

        void Current_Activated(object sender, WindowActivatedEventArgs e)
        {
            // tesztelés miatt
            if (!System.Diagnostics.Debugger.IsAttached)
            {
                if (e.WindowActivationState == CoreWindowActivationState.Deactivated)
                    Cancel();
            }
        }

        void window_PointerCaptureLost(CoreWindow sender, PointerEventArgs args)
        {
        }

        void window_SizeChanged(CoreWindow sender, WindowSizeChangedEventArgs args)
        {
        }

        void Current_Suspending(object sender, Windows.ApplicationModel.SuspendingEventArgs e)
        {
            Cancel();
        }

        void DisplayInformation_DisplayContentsInvalidated(DisplayInformation sender, object args)
        {
        }

        void currentDisplayInformation_OrientationChanged(DisplayInformation sender, object args)
        {
        }

        void currentDisplayInformation_DpiChanged(DisplayInformation sender, object args)
        {
        }

        void window_VisibilityChanged(CoreWindow sender, VisibilityChangedEventArgs args)
        {
            //Cancel();
        }

        internal void Cancel()
        {
            if (actualTest >= 0 && actualTest < tests.Count)
                tests[actualTest].Cancel();
        }
        #endregion

        #region Test Events
        void test_OnCancelled()
        {
            Debug.WriteLine("CANCELLED");

            // game over
            HandleOnSessionEnd();
        }

        void test_OnSkipped()
        {
            Debug.WriteLine("SKIP");
            //NextTest(); // ends in wrong behaviour
            // fallback to cancel
            test_OnCancelled();
        }

        async void test_OnTestFinished()
        {
            await SaveToLocal();
            Debug.WriteLine("FINISHED");
            NextTest();
        }

        private async Task SaveToLocal()
        {
            PlannedTest test = tests[actualTest];

            bool lastTest = false;
            // check last test
            if (tests.Count > 0 && (actualTest + 1) < tests.Count)
            {
                lastTest = false;
            }
            else
            {
                lastTest = true;
            }

            //write result to file
            try
            {
                StorageFile resultFile = await _resultFolder.CreateFileAsync(test.TestData.TestId + ".json");
                using (IRandomAccessStream resultRandomAccessStream = await resultFile.OpenAsync(FileAccessMode.ReadWrite))
                {
                    using (Stream result_stream = resultRandomAccessStream.AsStreamForWrite())
                    {
                        using (StreamWriter swriter = new StreamWriter(result_stream))
                        {
                            swriter.Write(test.Result);
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                throw;
            }

            if (lastTest)
            {
                HandleOnSessionEnd();
            }
        }

        void test_OnTestInitialized(bool successed, string result)
        {
            var uiAsyncCall = this.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                IsLoading = false;
            });

            if (successed)
            {
                Debug.WriteLine("INITIALIZED FINISHED");
            }
            else
            {
                Debug.WriteLine("INITIALIZED ERROR");
            }
        }

        void test_OnProgressChanged(double progress)
        {
            var uiAsyncCall = this.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                ProgressValue = progress * 100;
            });

            Debug.WriteLine(progress);
        }
        #endregion

        public async Task PrepareTest(List<TestData> data)
        {
            // first increment! start at 0
            actualTest = -1;

            Size native_resolution;
            double scaleFactor = 1;

            //calculate native resolution
            scaleFactor = DisplayInformation.GetForCurrentView().RawPixelsPerViewPixel;
            native_resolution.Width = (Int32)(Window.Current.Bounds.Height * scaleFactor);
            native_resolution.Height = (Int32)(Window.Current.Bounds.Width * scaleFactor);

            DateTime currentTime = DateTime.Now;

            foreach (TestData td in data)
            {
                PlannedTest test = new PlannedTest(td);

                test.NativeResolution = native_resolution;
                test.OnProgressChanged += test_OnProgressChanged;
                test.OnTestInitialized += test_OnTestInitialized;
                test.OnTestFinished += test_OnTestFinished;
                test.OnSkipped += test_OnSkipped;
                test.OnCancelled += test_OnCancelled;
                test.OnConnectionError += test_OnConnectionError;
                tests.Add(test);
            }


            Windows.Security.ExchangeActiveSyncProvisioning.EasClientDeviceInformation CurrentDeviceInfor = new Windows.Security.ExchangeActiveSyncProvisioning.EasClientDeviceInformation();

            string friendlyName = CurrentDeviceInfor.SystemSku;

            StorageFolder sessionsFolder = null;
            bool need_create_folder = false;

            try
            {
                sessionsFolder = await ApplicationData.Current.TemporaryFolder.GetFolderAsync(friendlyName);
            }
            catch (Exception ex)
            {
                need_create_folder = true;
            }

            if (need_create_folder)
            {
                sessionsFolder = await ApplicationData.Current.TemporaryFolder.CreateFolderAsync(friendlyName);
            }

            string folder_name = string.Format("{0}_{1}_{2}_{3}_{4}_{5}", currentTime.Year, currentTime.Month, currentTime.Day, currentTime.Hour, currentTime.Minute, currentTime.Second);
            try
            {
                //StorageFolder folder = ApplicationData.Current.LocalFolder;
                _resultFolder = await sessionsFolder.CreateFolderAsync(folder_name);
            }
            catch (Exception ex)
            {

            }

            Debug.WriteLine("CREATED SESSION/TEST FOLDER");
        }

        private async void test_OnConnectionError(string message)
        {

        }

        public async Task NextTest()
        {
            actualTest++;

            if (tests.Count > 0 && actualTest < tests.Count)
            {
                await tests[actualTest].Init(contextFactory);
            }
            else
            {

            }
        }

        async Task saveStringToLocalFile(string filename, string content)
        {
            // saves the string 'content' to a file 'filename' in the app's local storage folder
            byte[] fileBytes = System.Text.Encoding.UTF8.GetBytes(content.ToCharArray());

            // create a file with the given filename in the local folder; replace any existing file with the same name
            StorageFile file = await Windows.Storage.ApplicationData.Current.LocalFolder.CreateFileAsync(filename, CreationCollisionOption.ReplaceExisting);

            // write the char array created from the content string into the file
            using (var stream = await file.OpenStreamForWriteAsync())
            {
                stream.Write(fileBytes, 0, fileBytes.Length);
            }
        }

        private async Task HandleOnSessionEnd()
        {
            try
            {
                await saveStringToLocalFile(Parameters.SessionId, "Windows Phone");
            }
            catch (Exception ex)
            {
                //session is already done
            }
            Application.Current.Exit();
        }

        internal void Resize(Size size)
        {
            if (actualTest >= 0 && actualTest < tests.Count)
            {
                if (tests[actualTest].Status != TestStatus.CANCELLED)
                {
                    contextFactory.SetLogicalSize(size);
                }
            }
            else
            {
                contextFactory.SetLogicalSize(size);
            }
        }
    }
}
