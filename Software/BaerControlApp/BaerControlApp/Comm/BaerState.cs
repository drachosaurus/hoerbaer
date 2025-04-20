using CommunityToolkit.Mvvm.ComponentModel;

namespace BaerControlApp.Comm;

public class BaerState
{
    public DevicePower Power { get; set; } = new DevicePower();
    
    public PlayingInfo PlayingInfo { get; set; } = new PlayingInfo();
}

public partial class DevicePower : ObservableObject
{
    [ObservableProperty] private bool batteryPresent;
    [ObservableProperty] private bool charging;
    [ObservableProperty] private float batteryVoltage;
    [ObservableProperty] private float batteryPercentage;
}

public partial class PlayingInfo : ObservableObject
{
    [ObservableProperty] private HoerBaer.Ble.PlayerState state;
    [ObservableProperty] private int slotActive;
    [ObservableProperty] private int fileIndex;
    [ObservableProperty] private int fileCount;
    [ObservableProperty] private int currentTime;
    [ObservableProperty] private int duration;
    [ObservableProperty] private int volume;
    [ObservableProperty] private int maxVolume;
}