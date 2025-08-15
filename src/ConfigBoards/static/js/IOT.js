document.addEventListener("DOMContentLoaded", () => {
    const toggleButton = document.getElementById("toggle-button"); // The hamburger menu button
    const panelLayer = document.getElementById("Panel_Layer"); // The sidebar menu
    const closeButton = document.getElementById("close-button"); // The close button inside the menu
  
    // Add click event listener to the toggle button
    toggleButton.addEventListener("click", (event) => {
      event.preventDefault(); // Prevent default action of the link
  
      // Remove 'close' class if it exists, then add 'open'
      panelLayer.classList.remove("open", "close");
      panelLayer.classList.add("open");
    });
  
    // Add click event listener to the close button
    closeButton.addEventListener("click", (event) => {
      event.preventDefault(); // Prevent default action of the link
  
      // Remove 'open' class if it exists, then add 'close'
      panelLayer.classList.remove("open");
      panelLayer.classList.add("close");
    });
  
    // Optional: Close the menu if clicked outside
    document.addEventListener("click", (event) => {
      if (
        panelLayer.classList.contains("open") && // Menu is open
        !panelLayer.contains(event.target) && // Click is outside the menu
        !toggleButton.contains(event.target) // Click is outside the toggle button
      ) {
        panelLayer.classList.remove("open");
        panelLayer.classList.add("close");
      }
    });
  });
  
  