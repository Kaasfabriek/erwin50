<?php

require "database.php";
$database = Database::getInstance();
$database->prepare("DELETE berichten FROM berichten;");
$database->execute();