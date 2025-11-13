using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace System
{
    public class SystemException : Exception
    {
        public SystemException(string Message)
            : base(Message)
        {
        }

        public SystemException(string Message, Exception innerException)
            : base(Message, innerException)
        {
        }
    }

    public class ApplicationException : Exception
    {
        public ApplicationException(string Message)
            : base(Message)
        {
        }

        public ApplicationException(string Message, Exception innerException)
            : base(Message, innerException)
        {
        }
    }
}

namespace Shared.Helper
{
    static public class Utils
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

        public static bool IsAny<T>(this IEnumerable<T> data)
        {
            return data != null && data.Any();
        }
    }

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

    public static class SizeExtensions
    {
        public static Windows.Foundation.Size Swap(this Windows.Foundation.Size size)
        {
            return new Windows.Foundation.Size(size.Height, size.Width);
        }
    }
}
