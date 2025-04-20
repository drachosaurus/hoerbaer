using Microsoft.Maui.Controls;
using Microsoft.Maui.Graphics;
using System;

namespace BaerControlApp.Controls;

public class RssiIndicator : GraphicsView
{
    // Plugin.BLE typically works with RSSI values from -100 (weak) to -30 (strong)
    public static readonly BindableProperty RssiValueProperty =
        BindableProperty.Create(nameof(RssiValue), typeof(int), typeof(RssiIndicator), -100, propertyChanged: OnRssiValueChanged);

    // These are the standard RSSI ranges used in BLE applications including Plugin.BLE
    public static readonly BindableProperty MaxRssiProperty =
        BindableProperty.Create(nameof(MaxRssi), typeof(int), typeof(RssiIndicator), -30, propertyChanged: OnRssiValueChanged);

    public static readonly BindableProperty MinRssiProperty =
        BindableProperty.Create(nameof(MinRssi), typeof(int), typeof(RssiIndicator), -100, propertyChanged: OnRssiValueChanged);

    public static readonly BindableProperty ArcCountProperty =
        BindableProperty.Create(nameof(ArcCount), typeof(int), typeof(RssiIndicator), 5, propertyChanged: OnRssiValueChanged);

    public static readonly BindableProperty ArcThicknessProperty =
        BindableProperty.Create(nameof(ArcThickness), typeof(float), typeof(RssiIndicator), 10f, propertyChanged: OnRssiValueChanged);

    public static readonly BindableProperty ArcSpacingProperty =
        BindableProperty.Create(nameof(ArcSpacing), typeof(float), typeof(RssiIndicator), 3f, propertyChanged: OnRssiValueChanged);

    // Colors that correspond to Plugin.BLE signal strength categories
    public static readonly BindableProperty StrongSignalColorProperty =
        BindableProperty.Create(nameof(StrongSignalColor), typeof(Color), typeof(RssiIndicator), Colors.Green, propertyChanged: OnRssiValueChanged);

    public static readonly BindableProperty MediumSignalColorProperty =
        BindableProperty.Create(nameof(MediumSignalColor), typeof(Color), typeof(RssiIndicator), Colors.Orange, propertyChanged: OnRssiValueChanged);

    public static readonly BindableProperty WeakSignalColorProperty =
        BindableProperty.Create(nameof(WeakSignalColor), typeof(Color), typeof(RssiIndicator), Colors.Red, propertyChanged: OnRssiValueChanged);

    public static readonly BindableProperty ShowRssiTextProperty =
        BindableProperty.Create(nameof(ShowRssiText), typeof(bool), typeof(RssiIndicator), true, propertyChanged: OnRssiValueChanged);

    public int RssiValue
    {
        get => (int)GetValue(RssiValueProperty);
        set => SetValue(RssiValueProperty, value);
    }

    public int MaxRssi
    {
        get => (int)GetValue(MaxRssiProperty);
        set => SetValue(MaxRssiProperty, value);
    }

    public int MinRssi
    {
        get => (int)GetValue(MinRssiProperty);
        set => SetValue(MinRssiProperty, value);
    }

    public int ArcCount
    {
        get => (int)GetValue(ArcCountProperty);
        set => SetValue(ArcCountProperty, value);
    }

    public float ArcThickness
    {
        get => (float)GetValue(ArcThicknessProperty);
        set => SetValue(ArcThicknessProperty, value);
    }

    public float ArcSpacing
    {
        get => (float)GetValue(ArcSpacingProperty);
        set => SetValue(ArcSpacingProperty, value);
    }

    public Color StrongSignalColor
    {
        get => (Color)GetValue(StrongSignalColorProperty);
        set => SetValue(StrongSignalColorProperty, value);
    }

    public Color MediumSignalColor
    {
        get => (Color)GetValue(MediumSignalColorProperty);
        set => SetValue(MediumSignalColorProperty, value);
    }

    public Color WeakSignalColor
    {
        get => (Color)GetValue(WeakSignalColorProperty);
        set => SetValue(WeakSignalColorProperty, value);
    }

    public bool ShowRssiText
    {
        get => (bool)GetValue(ShowRssiTextProperty);
        set => SetValue(ShowRssiTextProperty, value);
    }

    public RssiIndicator()
    {
        Drawable = new RssiDrawable(this);
        HeightRequest = 150;
        WidthRequest = 150;
    }

    private static void OnRssiValueChanged(BindableObject bindable, object oldValue, object newValue)
    {
        if (bindable is RssiIndicator indicator)
        {
            indicator.Invalidate();
        }
    }

    /// <summary>
    /// Gets the signal strength category based on RSSI values used in Plugin.BLE
    /// </summary>
    public static SignalStrength GetSignalStrength(int rssi)
    {
        // These thresholds match Plugin.BLE's typical categorization
        if (rssi >= -67)
            return SignalStrength.High;
        if (rssi >= -80)
            return SignalStrength.Medium;
        return SignalStrength.Low;
    }

    public enum SignalStrength
    {
        Low,
        Medium,
        High
    }

    private class RssiDrawable : IDrawable
    {
        private readonly RssiIndicator _indicator;

        public RssiDrawable(RssiIndicator indicator)
        {
            _indicator = indicator;
        }

        public void Draw(ICanvas canvas, RectF dirtyRect)
        {
            float centerX = dirtyRect.Width / 2;
            float centerY = dirtyRect.Height;
            float radius = Math.Min(dirtyRect.Width, dirtyRect.Height) * 0.8f;

            // Normalize RSSI value - handle 0 value (no signal/not connected) as minimum
            float normalizedValue = _indicator.RssiValue == 0 ? 
                0 : 
                NormalizeRssi(_indicator.RssiValue, _indicator.MinRssi, _indicator.MaxRssi);
            
            int activeBars = (int)Math.Ceiling(normalizedValue * _indicator.ArcCount);

            // Get current signal strength category
            SignalStrength currentStrength = GetSignalStrength(_indicator.RssiValue);

            // Draw arcs
            for (int i = 0; i < _indicator.ArcCount; i++)
            {
                float arcRadius = radius - (i * (_indicator.ArcThickness + _indicator.ArcSpacing));
                bool isActive = i < activeBars;

                Color arcColor = GetArcColor(i, _indicator.ArcCount, isActive, currentStrength);
                canvas.StrokeColor = arcColor;
                canvas.StrokeSize = _indicator.ArcThickness;

                float startAngle = 180;
                float endAngle = 360;
                canvas.DrawArc(centerX - arcRadius, centerY - arcRadius, arcRadius * 2, arcRadius * 2, startAngle, endAngle, false, false);
            }

            // Draw RSSI value text
            if (_indicator.ShowRssiText)
            {
                string rssiText = _indicator.RssiValue == 0 ? 
                    "No Signal" : 
                    $"{_indicator.RssiValue} dBm";
                
                canvas.FontColor = Colors.Black;
                canvas.FontSize = 14;
                
                var textSize = canvas.GetStringSize(rssiText, Microsoft.Maui.Graphics.Font.Default, 14);
                canvas.DrawString(rssiText, centerX - textSize.Width / 2, centerY - radius / 4, HorizontalAlignment.Left);
            }
        }

        private float NormalizeRssi(int rssi, int minRssi, int maxRssi)
        {
            // Ensure the RSSI is within bounds
            int clampedRssi = Math.Clamp(rssi, minRssi, maxRssi);
            
            // Convert to a 0-1 range
            float normalizedValue = (float)(clampedRssi - minRssi) / (maxRssi - minRssi);
            return normalizedValue;
        }

        private Color GetArcColor(int arcIndex, int totalArcs, bool isActive, SignalStrength currentStrength)
        {
            if (!isActive)
            {
                return Colors.Gray.WithAlpha(0.3f);
            }

            // Use the appropriate color based on the current signal strength category
            return currentStrength switch
            {
                SignalStrength.High => _indicator.StrongSignalColor,
                SignalStrength.Medium => _indicator.MediumSignalColor,
                _ => _indicator.WeakSignalColor
            };
        }
    }

}
