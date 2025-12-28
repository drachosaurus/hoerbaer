import { Route, Routes } from "react-router";
import Home from "./home/Home";
import { NavLinks } from "./nav-links";

function App() {
  return (
    <div className="sm:px-4 lg:px-12">
      <nav>
        <div className="mx-auto max-w-7xl px-4">
          <div className="flex h-16 items-center justify-between">
            <div className="flex items-center">
              <NavLinks />
            </div>
          </div>
        </div>
      </nav>
      <main>
        <div className="mx-auto max-w-7xl py-6">
          <Routes>
            <Route path="/" element={<Home />} />
          </Routes>
        </div>
      </main>
    </div>
  );
}

export default App;
