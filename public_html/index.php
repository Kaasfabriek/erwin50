<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8">
    <title>Erwin 50 jaar!!</title>
  <!-- Latest compiled and minified CSS -->
  <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css" integrity="sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u" crossorigin="anonymous">
    <link rel="stylesheet" href="style.css" type="text/css">
  
  </head>
  <body>
  <center><div class="jumbotron text-center"><h1>Feliciteer Erwin met zijn 50ste verjaardag!</h1>
      <h2>Stuur hem een bericht via lora!</h2>
      </div></center>
      <center><p>
      <form method="post" action="bericht.php">
          <div class="form-group">
              <label for="boodschap">Wat wil je tegen Erwin zeggen? (max 64 tekens)</label>
                <input type="text" name="boodschap1" maxlength="64" class="form-control boodschap">
                
          </div>
          <button type="submit" class="btn btn-success">Submit</button>
      </form>
      </p></center>
  <center>
      <div class="jumbotron text-center">
        <h2>Welke berichten zijn nog niet opgehaald?</h2>
        <p>
        <ul>
            
            <?php
            require "database.php";
            $database = Database::getInstance();
            $database->prepare("SELECT id, messenge FROM berichten ORDER BY id ASC");
            $database->execute();
            $results = $database->getAll();
            foreach($results as $result) {
                $result['messenge'] = htmlspecialchars($result['messenge']);
                ?>
            <li><?=$result['messenge']?></li>
            <?php
            }
            ?>
        </ul>
        </p>
    </div>
  </center>
  <script src="//ajax.googleapis.com/ajax/libs/jquery/1.7.2/jquery.min.js" type="text/javascript"></script>
  </body>
</html>