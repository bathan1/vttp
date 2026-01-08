.mode box
.load ./libvttp

CREATE VIRTUAL TABLE patients USING vttp (
    "resourceType" TEXT,
    id INT,
    gender TEXT
);

SELECT * FROM patients WHERE url = 'https://r4.smarthealthit.org/Patient' and body = '$.entry[*].resource';
