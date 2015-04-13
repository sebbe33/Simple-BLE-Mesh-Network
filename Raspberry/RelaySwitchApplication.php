<?php
require_once 'MeshNodeApplicationImpl.php';
require_once 'MeshNodeApplicationOperationCodeImpl.php';

class RelaySwitchApplication extends MeshNodeApplicationImpl {
    const RELAY_SWITCH_CODE = 0x01;
    const OP_SET_STATUS = 0x01;
    const OP_GET_STATUS_REQUEST = 0x02;
    const OP_GET_STATUS_RESPONSE = 0x03;
    const OP_GET_MESSAGE_MAX_LENGTH = 20;
    
    public function __construct() {
        parent::__construct($this::RELAY_SWITCH_CODE, "Relay Switch", array(
            $this::OP_SET_STATUS => new MeshNodeApplicationOperationCodeImpl($this::OP_SET_STATUS, "Set the status"),
            $this::OP_GET_STATUS_REQUEST => new MeshNodeApplicationOperationCodeImpl($this::OP_GET_STATUS_REQUEST, "Get status request"),
            $this::OP_GET_STATUS_RESPONSE => new MeshNodeApplicationOperationCodeImpl($this::OP_GET_STATUS_RESPONSE, "Get status response"),
        ));
    }
    
    public function constructMessage($OPCode, $message) {
        $msg = parent::constructMessage($OPCode, $message);
       
        if(strlen($msg) > $this::OP_GET_MESSAGE_MAX_LENGTH) {
            return false;
        }
        
        return $msg;
    }
}