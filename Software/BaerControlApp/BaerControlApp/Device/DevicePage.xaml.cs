namespace BaerControlApp.Device;

[QueryProperty(nameof(DeviceId), "deviceId")]
public partial class DevicePage : DevicePageBase<DeviceViewModel>
{
    public DevicePage(IServiceScopeFactory serviceScopeFactory) : base(serviceScopeFactory)
    {
        InitializeComponent();
    }
}