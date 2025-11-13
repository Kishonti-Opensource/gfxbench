using System;
using System.Collections.Generic;
using System.Text;

namespace testfw_app.Helper
{
    public static class DateTimeHelper
    {
        public static TimeSpan DateTimeToTimeSpan(DateTime? ts)
        {
            if (!ts.HasValue)
            {
                return TimeSpan.Zero;
            }
            else
            {
                return new TimeSpan(0, ts.Value.Hour, ts.Value.Minute, ts.Value.Second, ts.Value.Millisecond);
            }
        }
    }
}
