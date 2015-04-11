<?php

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
}
