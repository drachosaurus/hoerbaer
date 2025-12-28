import { ChangeEvent, FormEvent, useState } from "react";
import Page from "../components/Page";
import TextBox from "../components/TextBox";
import { WifiConnectionState } from "../api/WifiConnectionState";
import ContentGroup from "../components/ContentGroup";
import PrimaryButton from "../components/PrimaryButton";
import ButtonRow from "../components/ButtonRow";
import {
  useGetWifiStatusQuery,
  useSetWifiCredentialsMutation,
  useSetWifiOffWhenEyesClosedMutation
} from "../api/deviceApi";
import SmallButton from "../components/SmallButton";

export default function Wifi() {
  const { data, isLoading, refetch } = useGetWifiStatusQuery();
  const [updateWifiCredentials] = useSetWifiCredentialsMutation();
  const [setWifiOffWhenEyesClosed] = useSetWifiOffWhenEyesClosedMutation();
  const [formData, setFormData] = useState({ ssid: "", key: "" });

  const handleInputChange = (e: ChangeEvent<HTMLInputElement>) => {
    const { name, value } = e.target;
    setFormData({ ...formData, [name]: value });
    console.log("formData", formData);
  };

  const handleSubmit = async (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault();
    await updateWifiCredentials({ ssid: formData.ssid || "", key: formData.key || "" });
    refetch();
  };

  const setOffWhenEyesClosed = async (offWhenEyesClosed: boolean) => {
    await setWifiOffWhenEyesClosed({ offWhenEyesClosed: offWhenEyesClosed });
    refetch();
  };

  const humanizeStatus = (state: WifiConnectionState) => {
    switch (state) {
      case WifiConnectionState.Connecting:
        return "Connecting";
      case WifiConnectionState.ConnectedStation:
        return "Connected (STA)";
      case WifiConnectionState.Standalone:
        return "Standalone";
      case WifiConnectionState.Disconnected:
        return "Disconnected";
      default:
        return "Unknown";
    }
  };

  return (
    <Page title="Wifi">
      <ContentGroup title="Status">
        {isLoading && <>Loading...</>}
        {data && (
          <>
            Connection: {data.ssid} - {humanizeStatus(data.connectionState)}
            <br />
            Configured SSID: {data.configuredSsid}
            <br />
            IP Address: {data.ip}
            <br />
            MAC Address: {data.mac}
            <br />
            RSSI: {data.rssi}
          </>
        )}
        <ButtonRow>
          <PrimaryButton onClick={refetch}>Refresh</PrimaryButton>
        </ButtonRow>
      </ContentGroup>
      <ContentGroup title="Auto off">
        {isLoading && <>Loading...</>}
        {data && (
          <>
            {data.offWhenEyesClosed ? "WLAN Off when eyes closed" : "WLAN always on"}
            <br />
            <SmallButton onClick={() => setOffWhenEyesClosed(!data.offWhenEyesClosed)}>Change</SmallButton>
          </>
        )}
      </ContentGroup>
      <h2 className="text-base font-semibold leading-7 text-gray-900 mt-6">Config</h2>
      <form onSubmit={handleSubmit}>
        <div className="mt-2 grid grid-cols-1 gap-x-6 gap-y-8 sm:grid-cols-6">
          <TextBox id="ssid" label="SSID" onChange={handleInputChange} />
          <TextBox type="password" id="key" label="Key" onChange={handleInputChange} />
        </div>
        <ButtonRow>
          <PrimaryButton type="submit">Save</PrimaryButton>
        </ButtonRow>
      </form>
    </Page>
  );
}
