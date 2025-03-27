<?php

include_once "assets/class.php";

// Define the new 'air' object:
$air = new _main();
$air->__init__($conn);

# Get the current date and time.
$date = date("Y_m_d_H_i_s");

# Create the image file name. 
$img_file = "IMG_".$date;

// If FireBeetle ESP32 sends the collected air quality data parameters with the model detection result, save the received information to the given MySQL database table.
if(isset($_GET["no2"]) && isset($_GET["o3"]) && isset($_GET["wind_speed"]) && isset($_GET["temperature"]) && isset($_GET["humidity"]) && isset($_GET["model_result"])){
	if($air->insert_new_data($date, $_GET["no2"], $_GET["o3"], $_GET["wind_speed"], $_GET["temperature"], $_GET["humidity"], $img_file.".jpg", $_GET["model_result"])){
		echo "Air Quality Data Saved to the Database Successfully!";
	}else{
		echo "Database Error!";
	}
}

// If FireBeetle ESP32 transfers a surveillance image (footage) to update the server, save the received raw image as a TXT file to the env_notifications folder.
if(!empty($_FILES["captured_image"]['name'])){
	// Image File:
	$captured_image_properties = array(
	    "name" => $_FILES["captured_image"]["name"],
	    "tmp_name" => $_FILES["captured_image"]["tmp_name"],
		"size" => $_FILES["captured_image"]["size"],
		"extension" => pathinfo($_FILES["captured_image"]["name"], PATHINFO_EXTENSION)
	);
	
    // Check whether the uploaded file extension is in the allowed file formats.
	$allowed_formats = array('jpg', 'png', 'txt');
	if(!in_array($captured_image_properties["extension"], $allowed_formats)){
		echo 'FILE => File Format Not Allowed!';
	}else{
		// Check whether the uploaded file size exceeds the 5 MB data limit.
		if($captured_image_properties["size"] > 5000000){
			echo "FILE => File size cannot exceed 5MB!";
		}else{
			// Save the uploaded file (image).
			move_uploaded_file($captured_image_properties["tmp_name"], "./env_notifications/".$img_file.".".$captured_image_properties["extension"]);
			echo "FILE => Saved Successfully!";
		}
	}
}

// Convert the recently saved raw image (TXT file) to a JPG file via the bmp_converter.py file.
$convert = shell_exec('python "C:\Users\kutlu\New E\xampp\htdocs\weather_station_data_center\env_notifications\bmp_converter.py"');
print($convert);

// After generating the JPG file, remove the recently saved TXT file from the server.
unlink("./env_notifications/".$img_file.".txt");

?>