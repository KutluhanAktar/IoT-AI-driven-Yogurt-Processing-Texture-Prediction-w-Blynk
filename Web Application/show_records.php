<?php

include_once "assets/class.php";

// Define the new 'air' object:
$air = new _main();
$air->__init__($conn);

// Obtain all data records from the database table and print them as table rows.
$date=[]; $no2=[]; $o3=[]; $temp=[]; $humd=[]; $wind=[]; $img=[]; $m_result=[];
list($date, $no2, $o3, $temp, $humd, $wind, $img, $m_result) = $air->get_data_records();
$records = "<tr><th>Date</th><th>NO2</th><th>O3</th><th>Temperature</th><th>Humidity</th><th>Wind Speed</th><th>Model Prediction</th><th>IMG</th></tr>";
for($i=0; $i<count($date); $i++){
	$records .= '<tr class="'.$m_result[$i].'">
				  <td>'.$date[$i].'</td>
				  <td>'.$no2[$i].'</td>
				  <td>'.$o3[$i].'</td>
				  <td>'.$temp[$i].'</td>
				  <td>'.$humd[$i].'</td>
				  <td>'.$wind[$i].'</td>
				  <td>'.$m_result[$i].'</td>
				  <td><button id="env_notifications/images/'.$img[$i].'">I</button></td>
			    </tr>
			   ';   
}	

// Get the latest surveillance image from the database table.
$latest_img = $img[0];

// Create a JSON object from the generated table rows and the latest surveillance image.
$result = array("records" => $records, "latest_img" => "env_notifications/images/".$latest_img);
$res = json_encode($result);

// Return the recently generated JSON object.
echo($res);

?>