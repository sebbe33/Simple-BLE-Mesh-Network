package simpleblemeshnetwork.exampleapp.network.application;

import java.util.Arrays;

/**
 * @author Sebastian Blomberg
 */
public enum MeshNodeApplicationImpl implements MeshNodeApplication {
    RELAY_SWITCH(
            (byte) 0x01, "Relay switch",
            "Turns on/off a relay connected to node", RelaySwitchOperationCodes.values()),
    NIS((byte) 0x02, "Node Information service", "Information about a node", null);

    private byte id;
    private String name, description;
    private ApplicationOperationCode[] OPCodes;

    private MeshNodeApplicationImpl(byte id, String name, String description,
                                    ApplicationOperationCode[] OPCodes) {
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

    @Override
    public ApplicationOperationCode[] getOperationCodes() {
        return OPCodes;
    }

    @Override
    public byte[] constructMessage(ApplicationOperationCode opCode, byte[] data) {
        byte[] newData = Arrays.copyOf(data, data.length + 1);
        // Copy the data
        System.arraycopy(data,0,newData,1,data.length);
        // Add the OP code
        newData[0] = opCode.getCode();

        return newData;
    }
}
