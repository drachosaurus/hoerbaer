/** @type {import('tailwindcss').Config} */
module.exports = {
  content: [
    "./src/**/*.{js,jsx,ts,tsx}",
  ],
  darkMode: "class",
  theme: {
    extend: {
      colors: {
        primary: "#8D6E63",
        secondary: "#D7CCC8",
        accent: "#FFAB40",
        wood: {
          light: "#EFEBE9",
          DEFAULT: "#A1887F",
          dark: "#5D4037",
          red: "#8D6E63",
        },
        "background-light": "#FDFBF7",
        "background-dark": "#2D2420",
      },
      fontFamily: {
        display: ["Fredoka", "sans-serif"],
        body: ["Nunito", "sans-serif"],
      },
      borderRadius: {
        DEFAULT: "1rem",
        'xl': '1.5rem',
        '2xl': '2rem',
      },
      boxShadow: {
        'wood': '4px 4px 10px rgba(0,0,0,0.15), -2px -2px 8px rgba(255,255,255,0.4)',
        'wood-pressed': 'inset 3px 3px 6px rgba(0,0,0,0.2), inset -2px -2px 5px rgba(255,255,255,0.3)',
        'floating': '0 20px 25px -5px rgba(0, 0, 0, 0.1), 0 10px 10px -5px rgba(0, 0, 0, 0.04)',
      }
    }
  },
  plugins: [
    require("@tailwindcss/forms")
  ]
}

