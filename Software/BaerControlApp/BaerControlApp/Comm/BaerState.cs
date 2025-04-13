using CommunityToolkit.Mvvm.ComponentModel;

namespace BaerControlApp.Comm;

public class BaerState
{
    public DevicePower Power { get; set; } = new DevicePower();
}

public partial class DevicePower : ObservableObject
{
    [ObservableProperty] private bool charging;

    [ObservableProperty] private float batteryVoltage;

    [ObservableProperty] private float batteryPercentage;
}
