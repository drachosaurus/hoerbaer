using BaerControlApp.Comm;

namespace BaerControlApp.Device;

public class TracklistViewModel : IDeviceViewModel
{
    public Task Initialize(DiscoveredDevice device)
    {
        return Task.CompletedTask;
    }
}