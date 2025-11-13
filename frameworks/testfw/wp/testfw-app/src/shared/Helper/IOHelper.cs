using System;
using System.Threading.Tasks;
using Windows.Storage;

namespace testfw_app.Helper
{
    public class IOHelper
    {
        static public async Task<StorageFolder> GetAssets()
        {
            StorageFolder InstallationFolder = Windows.ApplicationModel.Package.Current.InstalledLocation;
            StorageFolder AssetsFolder = await InstallationFolder.GetFolderAsync("Assets");
            return AssetsFolder;
        }

        static public string GetInstallationPath()
        {
            StorageFolder InstallationFolder = Windows.ApplicationModel.Package.Current.InstalledLocation;
            //StorageFolder InstallationFolder = ApplicationData.Current.LocalFolder;
            return InstallationFolder.Path;
        }

        static public StorageFolder GetLocalFolder()
        {
            return ApplicationData.Current.LocalFolder;
        }

        static public StorageFolder GetTempFolder()
        {
            return ApplicationData.Current.TemporaryFolder;
        }
    }
}