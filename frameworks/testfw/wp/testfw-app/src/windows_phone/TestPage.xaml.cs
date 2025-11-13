
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

using Windows.UI.Core;
using Windows.UI.Input;
using Windows.Graphics.Display;
using Windows.System.Threading;
using Windows.Phone.UI.Input;
using Windows.Storage;
using System.Threading.Tasks;
using Windows.UI.Xaml.Media.Animation;
using Windows.UI.Popups;
using System.Diagnostics;
using System.ComponentModel;
using Windows.System.Display;
using Windows.UI.ViewManagement;
using testfw_app.Model;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkID=390556

namespace testfw_app
{
    public sealed partial class TestPage : Page
    {
        private TestPageModel context;
        private DisplayRequest displayRequest;

        public TestPage()
        {
            this.InitializeComponent();

            swapChainPanel.SizeChanged += swapChainPanel_SizeChanged;

            context = new TestPageModel(Dispatcher, swapChainPanel);
            Loaded += TestPage_Loaded;
            
            DataContext = context;
        }

        
        void swapChainPanel_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            context.Resize(e.NewSize);
        }

        /// <summary>
        /// Invoked when this page is about to be displayed in a Frame.
        /// </summary>
        /// <param name="e">Event data that describes how this page was reached.
        /// This parameter is typically used to configure the page.</param>
        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            DisplayInformation.AutoRotationPreferences = DisplayOrientations.Portrait;
            HardwareButtons.BackPressed += HardwareButtons_BackPressed;

            if (displayRequest == null)
                displayRequest = new Windows.System.Display.DisplayRequest();

            displayRequest.RequestActive();

            StatusBar statusBar = Windows.UI.ViewManagement.StatusBar.GetForCurrentView();

            // Hide the status bar
            await statusBar.HideAsync();

            //Show the status bar
            //await statusBar.ShowAsync();

            context.Frame = this.Frame;
            await context.PrepareTest(e.Parameter as List<TestData>);
        }

        void TestPage_Loaded(object sender, RoutedEventArgs e)
        {
            context.NextTest();
        }

        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            DisplayInformation.AutoRotationPreferences = DisplayOrientations.Portrait;
            HardwareButtons.BackPressed -= HardwareButtons_BackPressed;

            if (displayRequest != null)
                displayRequest.RequestRelease(); 
        }

        public void HardwareButtons_BackPressed(object sender, BackPressedEventArgs e)
        {
            context.Cancel();
            e.Handled = true;
        }
    }
}
