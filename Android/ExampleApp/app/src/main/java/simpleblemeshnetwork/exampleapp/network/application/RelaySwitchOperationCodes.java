package simpleblemeshnetwork.exampleapp.network.application;

/**
 * @author Sebastian Blomberg
 */
public enum RelaySwitchOperationCodes implements ApplicationOperationCode {
    SET_STATUS((byte) 0x01, "Set the status of the relay switch"),
    GET_STATUS_REQUEST((byte) 0x02, "Requests a status report"),
    GET_STATUS_RESPONSE((byte) 0x03, "A status response containing the current status");

    private byte code;
    private String description;

    private RelaySwitchOperationCodes(byte code, String description) {
        this.code = code;
        this.description = description;
    }

    @Override
    public byte getCode() {
        return code;
    }

    @Override
    public String getOperationDescription() {
        return description;
    }
}
