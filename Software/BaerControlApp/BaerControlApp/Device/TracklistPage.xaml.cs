namespace BaerControlApp.Device;

[QueryProperty(nameof(DeviceId), "deviceId")]
public partial class TracklistPage : DevicePageBase<TracklistViewModel>
{
    public TracklistPage(IServiceScopeFactory serviceScopeFactory) : base(serviceScopeFactory)
    {
        InitializeComponent();
    }
}