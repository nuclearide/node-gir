var gir = require('../gir');
var gtk = gir.load('Gtk', '3.0');

gir.StartLoop();

module.exports = gtk;
