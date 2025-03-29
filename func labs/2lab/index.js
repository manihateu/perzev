"use strict";
const text = "Адрес: 999999, г. Ульяновск, ул. Ленина, 34. Другой адрес: 123456, г. Москва, ул. Тверская, 10.";
const regex = /\d{6},\sг\.\s[А-Яа-яёЁ\- ]+,\sул\.\s[А-Яа-яёЁ\- ]+,\s\d+/g;
const matches = text.match(regex);
console.log(matches);
const text1 = "Дроби: 3/4, 5/8, 12/7.";
const regex1 = /(\d+)\/(\d+)/g;
const result1 = text1.replace(regex1, "$2/$1");
console.log(result1); // "Дроби: 4/3, 8/5, 7/12."
