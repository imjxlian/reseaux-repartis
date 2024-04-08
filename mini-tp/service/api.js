const fetch = require('node-fetch');

const GOUV_API_URL = "https://data.enseignementsup-recherche.gouv.fr/api/explore/v2.1/catalog/datasets/fr-esr-principaux-etablissements-enseignement-superieur/exports/json?lang=fr&timezone=Europe/Berlin";
const FLIGHT_API_URL = "https://aviation-edge.com/v2/public";
const FLIGHT_API_KEY = "693142-6238dd";

const getCoord = async (sigle) => {
  try {
    const response = await fetch(GOUV_API_URL);
    const data = await response.json();
    const ipb = data.filter(function (data) {
      return data.sigle === sigle;
    });

    if (ipb.length === 0) {
      throw new Error("No data found for the given sigle");
    }

    const coordonnees = ipb[0].coordonnees;
    const lat = coordonnees.lat;
    const lon = coordonnees.lon;

    return { lat, lon };
  } catch (error) {
    console.error(error);
  }
};

const findAircraftsByCoord = async ({ lat, lon }, distance) => {
  try {
    const response = await fetch(`${FLIGHT_API_URL}/flights?key=${FLIGHT_API_KEY}&lat=${lat}&lng=${lon}&distance=${distance}`);
    const data = await response.json();
    console.log(data);
    return data;
  } catch (error) {
    console.error(error);
  }
}

function calculateDistance(lat1, lon1, lat2, lon2) {
  const radLat1 = (Math.PI * lat1) / 180;
  const radLon1 = (Math.PI * lon1) / 180;
  const radLat2 = (Math.PI * lat2) / 180;
  const radLon2 = (Math.PI * lon2) / 180;

  const dLon = radLon2 - radLon1;
  const dLat = radLat2 - radLat1;
  const a =
    Math.sin(dLat / 2) ** 2 +
    Math.cos(radLat1) * Math.cos(radLat2) * Math.sin(dLon / 2) ** 2;
  const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
  const distance = 6371 * c;

  return distance;
}

function findClosestAircraft(aircrafts, lat, lng) {
  let minDistance = Number.MAX_VALUE;
  let closestAircraft;

  if (aircrafts.length === 0 || aircrafts === null) {
    return null;
  }

  aircrafts.forEach((aircraft) => {
    const distance = calculateDistance(
      aircraft.geography.latitude,
      aircraft.geography.longitude,
      lat,
      lng
    );

    if (distance < minDistance) {
      minDistance = distance;
      closestAircraft = aircraft;
    }
  });

  return closestAircraft ?? null;
}

async function getAirlineName(iata) {
  try {
    const response = await fetch(`${FLIGHT_API_URL}/airlineDatabase?key=${FLIGHT_API_KEY}&codeIataAirline=${iata}`);
    const data = await response.json();
    return data[0].nameAirline;
  } catch (error) {
    console.error(error);
  }
}

module.exports = {
  getCoord,
  findAircraftsByCoord,
  findClosestAircraft,
  getAirlineName
};
