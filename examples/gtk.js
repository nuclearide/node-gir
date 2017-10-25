var gir = require('../gir');

const startTime = Date.now();
var gtk = gir.load('Gtk', '3.0');
const endTime = Date.now();
console.log(`loaded gtk bindings in: ${endTime - startTime}ms`)

gir.StartLoop();

module.exports = gtk;

