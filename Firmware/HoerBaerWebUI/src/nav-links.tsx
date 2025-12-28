import { NavLink, useLocation } from "react-router-dom";

export function NavLinks() {
  const classNamesActive = "bg-gray-900 text-white rounded-md px-3 py-2 text-sm";
  const classNamesPassive = "text-black hover:bg-gray-700 hover:text-white rounded-md px-3 py-2 text-sm";
  const { pathname } = useLocation();

  return (
    <div className="flex items-baseline space-x-4">
      <NavLink to="/" className={`${pathname === "/" ? classNamesActive : classNamesPassive}`}>
        Home
      </NavLink>
    </div>
  );
}
