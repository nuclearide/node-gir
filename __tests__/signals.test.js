const { Gtk } = require('../');

describe('signals', () => {
  test('should be called when emitted', (done) => {
    // this test will timeout if the signal handler is never called
    const button = new Gtk.Button();
    button.connect('clicked', () => {
      done();
    });
    button.clicked();
  });

  test('signal connections should return a number that can be used to remove them', () => {
    const button = new Gtk.Button();
    const connectionId = button.connect('clicked', () => undefined);
    expect(typeof(connectionId)).toEqual('number');
  });

  // TODO: write a test to ensure object.disconnect(connectionId) works
});
