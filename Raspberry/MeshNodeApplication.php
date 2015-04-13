<?php
require_once("MeshNodeApplicationOperationCode.php");

interface MeshNodeApplication {
    /**
     * Returns the id of the application
     * @return numeric Application's identification
     */
    public function getId();
    
    /**
     * Returns the name of the application
     * @return String Applications's name
     */
    public function getName();
    
    /**
     * Returns the available OP-Codes of the application
     * @return Array of MeshNodeApplicationOperationCode
     */
    public function getOperationCodes();
    
    /**
     * Constructs a message with the specified OP-Code.
     * @param MeshNodeApplicationOperationCode 
     *        <strong>or corresponding constant</strong>
     * @return String - The message with added OP-Code.
     */
    public function constructMessage($OPCode, $message);
}
