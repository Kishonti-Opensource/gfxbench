using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using Windows.Storage;
using Windows.System.Threading;
using Windows.UI.Core;
using Windows.Phone.System.Power;
using Windows.ApplicationModel.Background;
using Windows.Devices.Enumeration.Pnp;

namespace testfw_app.SystemInfo
{
    public class Battery
    {
        #region Instance
        private static volatile Battery _instance;
        private static object syncRoot = new Object();

        public static Battery Instance
        {
            get
            {
                if (_instance == null)
                {
                    lock (syncRoot)
                    {
                        if (_instance == null)
                        {
                            _instance = new Battery();
                        }
                    }
                }

                return _instance;
            }
        }

        private Battery()
        {
        }
        #endregion

        #region Properties
        public TimeSpan RemaningDiscargeTime
        {
            get
            {
                return Windows.Phone.Devices.Power.Battery.GetDefault().RemainingDischargeTime;
            }
        }

        public bool PowerSave
        {
            get
            {
                return PowerManager.PowerSavingModeEnabled; 
            }
        }

        public int BateryLevel
        {
            get
            {
                return Windows.Phone.Devices.Power.Battery.GetDefault().RemainingChargePercent;
            }
        }

        public bool IsCharging
        {
            get
            {
                //http://stackoverflow.com/questions/23344251/device-status-in-windows-phone-8-1
                return RemaningDiscargeTime.Days > 1000;
            }
        }

        public bool IsConnected
        {
            get
            {
                //http://stackoverflow.com/questions/23344251/device-status-in-windows-phone-8-1
                return RemaningDiscargeTime.Days > 1000;
            }
        }
        #endregion
    }
}
