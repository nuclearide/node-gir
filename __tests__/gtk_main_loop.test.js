const { Gtk } = require('../');

describe('Gtk main loop integration', () => {
  test('it should not block setTimeout from popping', (done) => {
    setTimeout(() => {
      Gtk.mainQuit();
      done();
    }, 0);
    Gtk.main();
  });

  test('it should not block nextTick from popping', (done) => {
    process.nextTick(() => {
      Gtk.mainQuit();
      done();
    }, 0);
    Gtk.main();
  });

  test('it should not block Promises from resolving', (done) => {
    const promise = new Promise(resolve => resolve('dummy-value'));
    promise.then((result) => {
      Gtk.mainQuit();
      expect(result).toEqual('dummy-value');
      done();
    });
    Gtk.main();
  })
});
