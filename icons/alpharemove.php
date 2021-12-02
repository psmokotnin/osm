<?php

//find icons/ios/*.png -exec php icons/alpharemove.php {} \;

$filename = $argv[1];
// Load a png image with alpha channel
$png = imagecreatefrompng($filename);

// Turn off alpha blending
imagealphablending($png, false);

// Set alpha flag
imagesavealpha($png, false);

imagepng($png, $filename);

imagedestroy($png);