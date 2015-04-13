<?php
require_once 'MeshNodeApplicationImpl.php';
require_once 'RelaySwitchApplication.php';

class MeshNodeApplications {
    const RELAY_SWITCH = RelaySwitchApplication::RELAY_SWITCH_CODE;

    
    private static $values;
    private static $hasBeenInitialized = false;

    public static function getApplication($id) {
        if(!MeshNodeApplications::$hasBeenInitialized) {
            MeshNodeApplications::initialize();
        }
        
        if(!isset(MeshNodeApplications::$values[$id])) {
            return null;
        }
        
        return MeshNodeApplications::$values[$id];
    }
    
    private static function initialize() {
        MeshNodeApplications::$values[MeshNodeApplications::RELAY_SWITCH] = new RelaySwitchApplication();
    }
}

?>
