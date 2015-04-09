package simpleblemeshnetwork.exampleapp.network.application;

/**
 * @author Sebastian Blomberg
 */
public enum MeshNodeApplicationImpl implements MeshNodeApplication {
    RELAY_SWITCH((byte) 0x01, "Relay switch", "Turns on/off a relay connected to node"),
    NIS((byte) 0x02, "Node Information service", "Information about a node");

    private byte id;
    private String name, description;

    private MeshNodeApplicationImpl(byte id, String name, String description) {
        this.id = id;
        this.name = name;
        this.description = description;
    }

    @Override
    public byte getId() {
        return id;
    }

    @Override
    public String getName() {
        return name;
    }

    @Override
    public String getDescription() {
        return description;
    }
}
