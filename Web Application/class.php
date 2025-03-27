<?php

// Define the _main class and its functions:
class _main {
	public $conn;
	
	public function __init__($conn){
		$this->conn = $conn;
	}
	
    // Database -> Insert Air Quality Data:
	public function insert_new_data($date, $no2, $o3, $wind_speed, $temperature, $humidity, $img, $model_result){
		$sql_insert = "INSERT INTO `entries`(`date`, `no2`, `o3`, `wind_speed`, `temperature`, `humidity`, `img`, `model_result`) 
		               VALUES ('$date', '$no2', '$o3', '$wind_speed', '$temperature', '$humidity', '$img', '$model_result');"
			          ;
		if(mysqli_query($this->conn, $sql_insert)){ return true; } else{ return false; }
	}
	
	// Retrieve all data records from the database table, transmitted by FireBeetle ESP32.
	public function get_data_records(){
		$date=[]; $no2=[]; $o3=[]; $temp=[]; $humd=[]; $wind=[]; $img=[]; $m_result=[];
		$sql_data = "SELECT * FROM `entries` ORDER BY `id` DESC";
		$result = mysqli_query($this->conn, $sql_data);
		$check = mysqli_num_rows($result);
		if($check > 0){
			while($row = mysqli_fetch_assoc($result)){
				array_push($date, $row["date"]);
				array_push($no2, $row["no2"]);
				array_push($o3, $row["o3"]);
				array_push($temp, $row["temperature"]);
				array_push($humd, $row["humidity"]);
				array_push($wind, $row["wind_speed"]);
				array_push($img, $row["img"]);
				array_push($m_result, $row["model_result"]);
			}
			return array($date, $no2, $o3, $temp, $humd, $wind, $img, $m_result);
		}else{
			return array(["Not Found!"], ["Not Found!"], ["Not Found!"], ["Not Found!"], ["Not Found!"], ["Not Found!"], ["surveillance.jpg"], ["Not Found!"]);
		}
	}
}

// Define database and server settings:
$server = array(
	"name" => "localhost",
	"username" => "root",
	"password" => "",
	"database" => "air_quality_aiot"
);

$conn = mysqli_connect($server["name"], $server["username"], $server["password"], $server["database"]);