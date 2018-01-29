const { load, Gtk } = require('../');

// $ sudo apt-get install libkeybinder-3.0-0 gir1.2-keybinder
const Keybinder = load('Keybinder', '3.0');
Keybinder.init(0);

const hotkey = '<Super>X';
const result = Keybinder.bind(hotkey, () => console.log('clicked'));
console.log(hotkey, result);

Gtk.main();
