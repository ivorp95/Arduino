<!DOCTYPE html>
<html>
<head>
    <meta http-equiv="refresh" content="5" >
    <link rel="stylesheet" type="text/css" href="style.css" media="screen"/>

	<title> OCITANJE PODATAKA SA SENZORA </title>

</head>

<body>

    <h1>PODACI SA SENZORA</h1>
<?php
$servername = "localhost";
$username = "if0_38170413";
$password = "ElisPatrik123";
$dbname = "if0_38170413_pdukic";

// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);
// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

$sql = "SELECT ID, senzor, location, temperatura, vlaznost, pritisak , reading_time FROM senzori ORDER BY id DESC"; /*select items to display from the sensordata table in the data base*/

echo '<table cellspacing="5" cellpadding="5">
      <tr> 
        <th>ID</th> 
        <th>Date $ Time</th> 
        <th>Senzor</thh> 
        <th>Lokacija</th> 
        <th>Temperatura &deg;C</th> 
        <th>Vlaznost &#37;</th>
        <th>Pritisak</th>       
      </tr>';
 
if ($result = $conn->query($sql)) {
    while ($row = $result->fetch_assoc()) {
        $row_id = $row["ID"];
        $row_reading_time = $row["reading_time"];
        $row_sensor = $row["senzor"];
        $row_location = $row["lokacija"];
        $row_temperatura = $row["temperatura"];
        $row_vlaznost = $row["vlaznost"]; 
        $row_pritisak = $row["pritisak"]; 
        
        // Uncomment to set timezone to - 1 hour (you can change 1 to any number)
       // $row_reading_time = date("Y-m-d H:i:s", strtotime("$row_reading_time - 1 hours"));
      
        // Uncomment to set timezone to + 4 hours (you can change 4 to any number)
        //$row_reading_time = date("Y-m-d H:i:s", strtotime("$row_reading_time + 4 hours"));
      
        echo '<tr> 
                <td>' . $row_id . '</td> 
                <td>' . $row_reading_time . '</td> 
                <td>' . $row_senzor . '</td> 
                <td>' . $row_lokacija . '</td> 
                <td>' . $row_temperatura . '</td> 
                <td>' . $row_vlaznost . '</td>
                <td>' . $row_pritisak . '</td> 
                
              </tr>';
    }
    $result->free();
}

$conn->close();
?> 
</table>

</body>
</html>

	</body>
</html>