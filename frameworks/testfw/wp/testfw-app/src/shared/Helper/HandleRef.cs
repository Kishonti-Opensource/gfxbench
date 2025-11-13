using System;
namespace System.Runtime.InteropServices
{
    public struct HandleRef
    {
        object wrapper;
        IntPtr handle;

        public HandleRef(object wrapper, IntPtr handle)
        {
            this.wrapper = wrapper;
            this.handle = handle;
        }

        public IntPtr Handle
        {
            get { return handle; }
        }

        public object Wrapper
        {
            get { return wrapper; }
        }

        public static implicit operator IntPtr(HandleRef value)
        {
            return value.Handle;
        }

        public static IntPtr ToIntPtr(HandleRef value)
        {
            return value.Handle;
            // Why did MS add a function for this?
        }
    }
}
