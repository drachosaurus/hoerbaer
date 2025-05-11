using BaerControlApp.Comm;

namespace BaerControlApp.Device;

public interface IDeviceViewModel
{
    public Task Initialize(DiscoveredDevice device);
}