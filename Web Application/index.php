<!DOCTYPE html>
<html>
<head>
<title>AI-assisted Air Quality Monitor</title>

<!--link to index.css-->
<link rel="stylesheet" type="text/css" href="assets/index.css"></link>

<!--link to favicon-->
<link rel="icon" type="image/png" sizes="36x36" href="assets/icon.png">

<!-- link to FontAwesome-->
<link rel="stylesheet" href="https://use.fontawesome.com/releases/v6.2.1/css/all.css">
 
<!-- link to font -->
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link href="https://fonts.googleapis.com/css2?family=Playfair+Display+SC:ital@1&display=swap" rel="stylesheet">

<!--link to jQuery script-->
<script src="https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js"></script>

</head>
<body>
<?php ini_set('display_errors',1);?> 
<h1><i class="fa-solid fa-lungs"></i> AI-assisted Air Quality Monitor</h1>
<div class="data">
<table>
<tr><th>Date</th><th>NO2</th><th>O3</th><th>Temperature</th><th>Humidity</th><th>Wind Speed</th><th>Model Prediction</th><th>IMG</th></tr>
<tr><td>X</td><td>X</td><td>X</td><td>X</td><td>X</td><td>X</td><td>X</td><td>X</td></tr>
</table>
</div>

<div class="surveillance">
<section>
<img id="latest_img" src="env_notifications/images/surveillance.jpg" alt="latest_surveillance_image"/>
<figcaption>Latest Surveillance IMG</figcaption>
</section>
<section>
<img id="selected_img" src="env_notifications/images/surveillance.jpg" alt="selected_surveillance_image"/>
<figcaption>Selected Surveillance Image</figcaption>
</section>
</div>

<!--Add the index.js file-->
<script type="text/javascript" src="assets/index.js"></script>

</body>
</html>