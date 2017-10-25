const { load, startLoop } = require('./addon');

const Gtk = load('Gtk', '3.0');
startLoop();
Gtk.init(0);

module.exports = Gtk;
