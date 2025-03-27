// Display the selected surveillance image (footage) on the screen.
$(".data").on("click", "button", (event) => {
	$("#selected_img").attr('src', event.target.id);
});

// Every 5 seconds, retrieve the HTML table rows generated from the database table rows to inform the user of the latest model detection results on ambient air quality.
setInterval(function(){
	$.ajax({
		url: "./show_records.php",
		type: "GET",
		success: (response) => {
			// Decode the obtained JSON object.
			const res = JSON.parse(response);
			// Assign HTML table rows.
			$(".data table").html(res.records);
			// Assign the latest surveillance image (footage).
			$("#latest_img").attr('src', res.latest_img);
		}
	});
}, 5000);