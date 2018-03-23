/*
 * Adapted to node-gir from the C version :
 * https://github.com/GNOME/gtk/blob/master/examples/builder.c
 */

const { Gtk } = require('../');

// Construct a GtkBuilder instance and load our UI description.
const builder = new Gtk.Builder();
builder.addFromFile('builder.ui');

// Connect signal handlers to the constructed widgets.
const win = builder.getObject('window');
win.connect('destroy', () => {
    Gtk.main_quit();
})
const button1 = builder.getObject('button1');
button1.connect('clicked', () => {
    console.log("Hello World");
})

const button2 = builder.getObject('button2');
button2.connect('clicked', () => {
    console.log("Hello World");
})

const quitButton = builder.getObject('quit');
quitButton.connect('clicked', () => {
    Gtk.main_quit();
})

Gtk.main();
