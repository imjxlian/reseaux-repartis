const mongoose = require('mongoose');
const Schema = mongoose.Schema;

const AircraftSchema = new Schema({
  icao: {
    type: String,
    required: true
  },
  airline: {
    type: String,
    required: true
  },
  schoolAcronym: {
    type: String,
    required: true
  },
  updated: {
    type: Date,
    default: Date.now
  }
});

module.exports = mongoose.model('Aircraft', AircraftSchema);