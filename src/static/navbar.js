const navMenu = document.querySelectorAll('.nav-menu > li');

navMenu.forEach(item => {
  item.addEventListener('mouseover', () => {
    item.querySelector('.sub-menu').style.display = 'block';
  });

  item.addEventListener('mouseout', () => {
    item.querySelector('.sub-menu').style.display = 'none';
  });
});
