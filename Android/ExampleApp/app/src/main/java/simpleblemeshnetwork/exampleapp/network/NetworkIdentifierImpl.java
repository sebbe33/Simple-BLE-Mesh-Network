package simpleblemeshnetwork.exampleapp.network;

/**
 * @author Sebastian Blomberg
 */
public class NetworkIdentifierImpl implements NetworkIdentifier{
    private short id;
    private String name;
    private ConnectionDetails connectionDetails= null;

    public NetworkIdentifierImpl(short id, String name) {
        this.id = id;
        this.name = name;
    }

    @Override
    public short getId() {
        return id;
    }

    @Override
    public String getName() {
        return name;
    }

    public ConnectionDetails getConnectionDetails() {
        return connectionDetails;
    }

    public void setConnectionDetails(ConnectionDetails connectionDetails) {
        this.connectionDetails = connectionDetails;
    }

    @Override
    public String toString() {
        return name;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        NetworkIdentifierImpl that = (NetworkIdentifierImpl) o;

        if (id != that.id) return false;
        if (!name.equals(that.name)) return false;

        return true;
    }

    @Override
    public int hashCode() {
        int result = (int) id;
        result = 31 * result + name.hashCode();
        return result;
    }

    public class ConnectionDetails {
        private String address;
        private int rssi;

        public ConnectionDetails(String address, int rssi) {
            this.address = address;
            this.rssi = rssi;
        }

        public String getAddress() {
            return address;
        }

        public int getRssi() {
            return rssi;
        }
    }
}
