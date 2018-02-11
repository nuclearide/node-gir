const { Gtk } = require('../');

const win = new Gtk.Window({
  type: Gtk.WindowType.TOPLEVEL,
  title: 'Node.JS Gtk Window'
});
win.setBorderWidth(10);

const button = new Gtk.Button();
button.setLabel('hallo, welt!');
win.add(button);

win.connect('destroy', () => {
  Gtk.mainQuit();
});

let clickedCount = 0;
button.connect('clicked', () => {
  button.setLabel(`clicked: ${clickedCount++} times`);
});

win.showAll();
Gtk.main();
