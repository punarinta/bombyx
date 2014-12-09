<?php

$xy = microtime(true);
$n = 10000;
$arr = ["foo" => "bar"];

while (--$n)
{
    $a = $arr["foo"];
}

// multiplication and formatting do not count as they go after the subtraction
echo ("\nExecution took: " . number_format((microtime(true) - $xy) * 1000000, 0, '.', '') . " microsecond(s).\n");
