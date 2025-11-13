using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Linq;
using Windows.UI.Popups;
using System.Text.RegularExpressions;

using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using WindowsPhoneApp;
using testfw_app.Helper;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace testfw_app.Model
{
    class AutoTest : INotifyPropertyChanged
    {
        private async Task<JObject> GetDescriptor(string id)
        {
            var fileName = id + ".json";
            string json = await FileHelper.ReadFile(await (await IOHelper.GetAssets()).GetFolderAsync("config"), fileName);
            return JObject.Parse(json);
        }

        static string ConvertStringArrayToString(string[] array)
        {
            //
            // Concatenate all the elements into a StringBuilder.
            //
            StringBuilder builder = new StringBuilder();
            foreach (string value in array)
            {
                builder.Append(value);
                builder.Append(' ');
            }
            return builder.ToString();
        }

        private bool ParseKeyValuePair<T>(string arg, ref Dictionary<string, T> rawt)
        {
            var raw_key_value = arg.Split(' ').Skip(1).ToArray();
            
            if (raw_key_value.Length == 1)
            {
                raw_key_value = raw_key_value[0].Split('=');
            }

            T value = default(T);
            string value_str = raw_key_value[1].Trim();
            if (typeof(T) == typeof(double))
            {
                value = (T)(object)double.Parse(value_str);
            }
            else if (typeof(T) == typeof(bool))
            {
                if ((value_str == "true") || (value_str == "1"))
                {
                    value = (T)(object)true;
                }
                else if ((value_str == "false") || (value_str == "0"))
                {
                    value = (T)(object)false;
                }
                else
                {
                    return false;
                }
            }
            else if (typeof(T) == typeof(String))
            {
                value = (T)(object)value_str;
            }
            else
            {
                return false;
            }

            rawt[raw_key_value[0].Trim()] = value;

            return true;
        }

        public async Task StartWithParameters()
        {
            bool successed = false;
            string parameters = null;
            try
            {
                parameters = await FileHelper.ReadFile(await IOHelper.GetTempFolder().GetFolderAsync("config"), "parameters.txt");
                successed = true;
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
            }

            if (!successed)
            {
                try
                {
                    parameters = await FileHelper.ReadFile(await IOHelper.GetLocalFolder().GetFolderAsync("config"), "parameters.txt");
                    successed = true;
                }
                catch (Exception ex)
                {
                    Debug.WriteLine(ex.Message);
                }
            }

            if (!successed)
            {
                MessageDialog msg = new MessageDialog("Please push the parameters.txt file to IsolatedStorage root folder.", "config file not found");
                await msg.ShowAsync();
                Application.Current.Exit();
            }

            string[] args = Regex.Split(parameters, @"(?=[@ ]--[^-]|[@ ]-[^-])");
            args = args.Select(arg => arg.Trim()).ToArray();

            // set the default values
            Parameters.Width = 0;
            Parameters.Height = 0;
            Parameters.IsDummyTest = false;
            Parameters.SessionId = "default_session";
            Parameters.TestList = null;
            Parameters.rawn = new Dictionary<string, double>();
            Parameters.raws = new Dictionary<string, string>();
            Parameters.rawz = new Dictionary<string, bool>();

            foreach (string arg in args)
            {
                if (arg.StartsWith("-t"))
                {
                    // parse testlist
                    var test_list = arg.Trim().Split(' ').Skip(1).ToArray();
                    for (int i = 0; i < test_list.Length; i++)
                    {
                        test_list[i] = test_list[i].Trim();
                    }
                    Parameters.TestList = (Parameters.TestList == null) ? test_list : Parameters.TestList.Concat(test_list).ToArray();
                }
                else if (arg.StartsWith("-s"))
                {
                    // parse session id
                    Parameters.SessionId = arg.Split(' ')[1].Trim();
                }
                else if (arg.StartsWith("-h"))
                {
                    // parse height
                    Parameters.Height = int.Parse(arg.Split(' ')[1].Trim());
                }
                else if (arg.StartsWith("-w"))
                {
                    // parse width
                    Parameters.Width = int.Parse(arg.Split(' ')[1].Trim());
                }
                else if (arg.StartsWith("--dummy_test"))
                {
                    // parse dummytest
                    Parameters.IsDummyTest = bool.Parse(arg.Split(' ')[1].Trim());
                }
                else if (arg.StartsWith("--ei"))
                {
                    // parse integer extra parameter
                    ParseKeyValuePair(arg, ref Parameters.rawn);
                }
                else if (arg.StartsWith("--ef"))
                {
                    // parse float extra parameter
                    ParseKeyValuePair(arg, ref Parameters.rawn);
                }
                else if (arg.StartsWith("--es"))
                {
                    // parse string extra parameter
                    ParseKeyValuePair(arg, ref Parameters.raws);
                }
                else if (arg.StartsWith("--ez"))
                {
                    // parse bool extra parameter 
                    ParseKeyValuePair(arg, ref Parameters.rawz);
                }
                else
                {
                    Debug.WriteLine("Unknown argument: {0}", arg);
                }
            }

            Debug.WriteLine("Width: {0}", Parameters.Width);
            Debug.WriteLine("Height: {0}", Parameters.Height);
            Debug.WriteLine("tests: {0}", ConvertStringArrayToString(Parameters.TestList));
            Debug.WriteLine("dummy test {0}", Parameters.IsDummyTest);

            List<TestData> tests = new List<TestData>();

            foreach (var t in Parameters.TestList)
            {
                JObject json = null;

                try
                {
                    json = await GetDescriptor(t);
                }
                catch (Exception)
                {
                    MessageDialog msg = new MessageDialog(string.Format("Test json file not found! {0}", t), "File not found!");
                    msg.ShowAsync();
                    continue;
                }

                tests.Add(new TestData
                {
                    Desc = json,
                    TestId = t
                });
            }

            if (tests.Count == 0)
            {
                Application.Current.Exit();
            }

            TestLists = parameters;
            await Task.Delay(5000);

            App.RootFrame.Navigate(typeof(TestPage), tests);
        }

        public event PropertyChangedEventHandler PropertyChanged;
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

        private String _testLists;
        public String TestLists
        {
            get { return _testLists; }
            set { _testLists = value; OnPropertyChanged("TestLists"); }
        }
    }
}
