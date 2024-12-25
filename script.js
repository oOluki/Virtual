document.getElementById('fileInput').addEventListener('change', (event) => {
  const file = event.target.files[0]; // Get the first selected file
  if (!file) {
    document.getElementById('output').textContent = 'No file selected.';
    return;
  }

  const reader = new FileReader(); // Create a FileReader object

  // Define what happens when the file is read
  reader.onload = () => {
    const arrayBuffer = reader.result; // Get the binary content
    const uint8Array = new Uint8Array(arrayBuffer); // Convert to a typed array

    // Format binary data for display
    const hexContent = Array.from(uint8Array)
      .map(byte => byte.toString(16).padStart(2, '0')) // Convert to hex
      .join(' ');

    document.getElementById('output').textContent = hexContent || 'File is empty.';
  };

  reader.onerror = () => {
    document.getElementById('output').textContent = 'Error reading file.';
  };

  reader.readAsArrayBuffer(file); // Read file as binary data
});
