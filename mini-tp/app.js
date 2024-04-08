const express = require('express');
const sqlite3 = require('sqlite3').verbose();
const { getCoord, findAircraftsByCoord, findClosestAircraft, getAirlineName } = require('./service/api');
const swaggerUi = require('swagger-ui-express');
const swaggerDocument = require('./service/swagger.json');

const app = express();
const db = new sqlite3.Database('mydatabase.db');

app.use('/api-docs', swaggerUi.serve, swaggerUi.setup(swaggerDocument));

db.serialize(() => {
  db.run("CREATE TABLE IF NOT EXISTS aircrafts (icao TEXT, airline TEXT, schoolAcronym TEXT, updated DATETIME)");
});

app.get('/', (req, res) => {
  res.send('Hello, world!');
});

app.get('/closest-aircraft/:sigle', async (req, res) => {
  const { sigle } = req.params;

  // Fonction pour vérifier la base de données ou appeler l'API
  const checkDatabaseOrUpdate = (sigle) => {
    db.get("SELECT * FROM aircrafts WHERE schoolAcronym = ? ORDER BY updated DESC LIMIT 1", [sigle], async (err, row) => {
      if (err) {
        return res.status(500).send("Error fetching data from the database");
      }
      const now = new Date();
      if (row && (now - new Date(row.updated)) < 60000) {
        console.log("Data found in the database");
        return res.json(row);
      } else {
        try {
          console.log("Data not found in the database, fetching from the API");
          const coord = await getCoord(sigle);
          const aircrafts = await findAircraftsByCoord(coord, 500);
          const closestAircraft = findClosestAircraft(aircrafts, coord.lat, coord.lon);

          console.log("Closest aircraft found: ", closestAircraft);

          // Mettre à jour la base de données avec les nouvelles données
          const updateStmt = db.prepare("INSERT INTO aircrafts (icao, airline, schoolAcronym, updated) VALUES (?, ?, ?, ?)");
          updateStmt.run(closestAircraft.flight.icaoNumber, closestAircraft.airline.iataCode, sigle, now.toISOString());
          updateStmt.finalize();

          // Fetch the Airline name from the icao24
          const airlineName = await getAirlineName(closestAircraft.airline.iataCode);

          // Return the data previously inserted
          return res.json({
            icao: closestAircraft.flight.icaoNumber,
            airline: airlineName,
            schoolAcronym: sigle,
            updated: now.toISOString()
          });
        } catch (apiError) {
          console.error(apiError);
          return res.status(500).send("An error occurred while fetching data from the API");
        }
      }
    });
  };

  checkDatabaseOrUpdate(sigle);
});

app.listen(3000, () => {
  console.log('Server is running on port 3000');
});
