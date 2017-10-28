<?php
$json = file_get_contents("php://input");
$json = explode(",",  $json);
$count = strlen($json);
$jsonx = array_map(function ($v) {
   return "0x" . str_pad(dechex($v), 2, "0", STR_PAD_LEFT);
}, $json);
$jsonv = "var bytex = [\n"  . chunk_split(implode(",", $jsonx), 80, "\n") . "];";
file_put_contents("assets/font.js", $jsonv);
header("Content-Type: application:json");
echo json_encode(["count" => $count]);