<?php
require_once 'MeshNodeApplicationImpl.php';

class MeshNodeApplications {
    const RELAY_SWITCH = 1;
    
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
        MeshNodeApplications::$values[MeshNodeApplications::RELAY_SWITCH] = new MeshNodeApplicationImpl(1, "Relay switch");
    }
}

?>
