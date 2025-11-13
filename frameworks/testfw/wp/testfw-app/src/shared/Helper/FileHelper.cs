using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Threading.Tasks;
using Windows.Storage;
using Windows.Storage.Streams;

namespace testfw_app.Helper
{
    public static class FileHelper
    {
        public static async Task<string> ReadFile(StorageFolder folder, string fileName)
        {
            try
            {
                StorageFile file = await folder.GetFileAsync(fileName);
                IRandomAccessStream fileRandomAccessStream = await file.OpenAsync(FileAccessMode.Read);
                StreamReader sreader = new StreamReader(fileRandomAccessStream.AsStreamForRead());

                return await sreader.ReadToEndAsync();
            }
            catch (Exception ex)
            {
                throw;
            }
        }
    }
}
