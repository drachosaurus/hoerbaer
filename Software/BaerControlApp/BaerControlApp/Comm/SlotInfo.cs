namespace BaerControlApp.Comm;

public class SlotInfo
{
    public string? Path { get; set; }
    public List<SlotTrackInfo> Files { get; set; } = new List<SlotTrackInfo>();
}

public class SlotTrackInfo
{
    public string? Path { get; set; }
    public string? Title { get; set; }
    public string? Artist { get; set; }
}