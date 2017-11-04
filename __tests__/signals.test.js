const { Gtk } = require('../');

describe('signals', () => {
  test('connect() returns a number', () => {
    const button = new Gtk.Button();
    const result = button.connect('clicked', () => undefined);
    expect(typeof (result)).toEqual('number');
  });

  test('disconnect() will stop the connected callback from being called', (done) => {
    const button = new Gtk.Button();
    const connection = button.connect('clicked', () => {
      done('the signal callback shouldn\'t have been called!');
    });
    button.disconnect(connection);
    button.clicked();
    setTimeout(() => done(), 0);
  });

  test('should be called when emitted', (done) => {
    // this test will timeout if the signal handler is never called
    const button = new Gtk.Button();
    button.connect('clicked', () => {
      done();
    });
    button.clicked();
  });

  test('a signal that passes parameters to it\'s callback should work', (done) => {
    const window = new Gtk.Window({ type: Gtk.WindowType.toplevel, title: "signal-test-window" });;
    const button = new Gtk.Button();
    button.connect('size-allocate', allocation => done());
    window.add(button);
    window.showAll();
  });

  test('signals that pass the instance to the closure should be the same object (i.e. ===)', (done) => {
    const entry = new Gtk.Entry();
    entry.connect('changed', (instance) => {
      expect(instance === entry).toBe(true);
      done();
    });
    entry.setText('dummy text');
  });
});
