/**
 * Guitar Tuner
 *
 * This is a node-gir port of GNOME's guitar tuner example from
 * <http://developer.gnome.org/gnome-devel-demos/unstable/guitar-tuner.js.html.en>
 */

const { load } = require('../');

const gtk = load('Gtk', '3.0');
const gst = load('Gst');

gtk.init(0);
gst.init(0);

const guitarwindow = new gtk.Window({
  type: gtk.WindowType.TOPLEVEL,
  title: 'Node.js Guitar Tuner'
});

guitarwindow.connect('destroy', () => {
  gtk.mainQuit();
  process.exit();
});

const guitarBox = new gtk.ButtonBox();

function playSound(frequency) {
  const pipeline = new gst.Pipeline({
    name: 'note'
  });
  const source = new gst.ElementFactory.make('audiotestsrc', 'source');
  const sink = new gst.ElementFactory.make('autoaudiosink', 'output');
  source.setProperty('freq', frequency);
  pipeline.add(source);
  pipeline.add(sink);
  source.link(sink);
  pipeline.setState(gst.State.PLAYING);
}

function addButton(tune, freq) {
  const button = new gtk.Button({
    label: tune
  });
  guitarBox.add(button);
  button.connect('clicked', () => {
    playSound(freq);
  });
}

const tunes = {
  E: 369.23,
  A: 440,
  D: 587.33,
  G: 783.99,
  B: 987.77,
  e: 1318.5
};

Object.keys(tunes).forEach((key) => {
  const tune = tunes[key];
  addButton(tune, tunes[tune]);
});

guitarwindow.add(guitarBox);

guitarBox.showAll();
guitarwindow.show();
gtk.main();
