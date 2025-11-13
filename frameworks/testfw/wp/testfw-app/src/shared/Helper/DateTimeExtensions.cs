using System;
using System.Collections.Generic;
using System.Text;

namespace GFXBenchAutoTest.Helper
{
    public static class DateTimeExtensions
    {
        public static DateTime DateTimeFromUnixLong(long value)
        {
            DateTime dt = new DateTime(1970, 1, 1, 0, 0, 0, 0, System.DateTimeKind.Utc);
            dt = dt.AddMilliseconds((long)value).ToLocalTime();
            return dt;
        }

        public static long ToUnixLong(this DateTime dt)
        {
            DateTime dtDateTime = new DateTime(1970, 1, 1, 0, 0, 0, 0, System.DateTimeKind.Utc);
            return (long)dt.ToUniversalTime().Subtract(dtDateTime).TotalMilliseconds;
        }
    }
}
