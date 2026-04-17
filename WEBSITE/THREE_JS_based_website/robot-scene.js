import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';
import { GLTFLoader } from 'three/addons/loaders/GLTFLoader.js';

// Kinematic lengths (used to position the invisible pivot groups)
const BASE_HEIGHT = 11.1;
const L1 = 10.50;
const L2 = 12.9;
const L3 = 16.25;

const robotParts = {};

export function initRobotScene(container) {
    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0x8ccbde);

    const camera = new THREE.PerspectiveCamera(45, container.clientWidth / container.clientHeight, 0.1, 500);
    camera.position.set(40, 30, 40);

    const renderer = new THREE.WebGLRenderer({ antialias: true });
    renderer.setSize(container.clientWidth, container.clientHeight);
    // Output encoding is important for GLTF colors to look correct
    renderer.outputColorSpace = THREE.SRGBColorSpace;
    container.appendChild(renderer.domElement);

    const controls = new OrbitControls(camera, renderer.domElement);
    controls.target.set(0, 20, 0);
    controls.update();

    // 1. Increase AmbientLight (lifts overall darkness globally)
    const hemiLight = new THREE.HemisphereLight(0xffffff, 0x444444, 1.5);
    hemiLight.position.set(0, 50, 0);
    scene.add(hemiLight);

    // 2. Increase Main DirectionalLight (acts as the sun)
    const dirLight = new THREE.DirectionalLight(0xfffae6, 3.5);
    dirLight.position.set(20, 40, 20);
    scene.add(dirLight);

    // 3. ADD a secondary Fill Light from the opposite side to soften shadows
    const fillLight = new THREE.DirectionalLight(0xcceeff, 1.5);
    fillLight.position.set(-20, 20, -20);
    scene.add(fillLight);

    scene.add(new THREE.GridHelper(80, 20, 0x444444, 0x222222));

    buildRobotKinematics(scene);
    loadGLTFModels();

    window.addEventListener('resize', () => {
        camera.aspect = container.clientWidth / container.clientHeight;
        camera.updateProjectionMatrix();
        renderer.setSize(container.clientWidth, container.clientHeight);
    });

    function animate() {
        requestAnimationFrame(animate);
        controls.update();
        renderer.render(scene, camera);
    }
    animate();
}

/**
 * Step 1: Build the invisible mathematical skeleton
 */
function buildRobotKinematics(scene) {
    // static base
    robotParts.base = new THREE.Group();
    scene.add(robotParts.base);

    // J1: Yaw Pivot
    robotParts.j1 = new THREE.Group();
    robotParts.j1.position.y = 5.610;
    scene.add(robotParts.j1);

    // J2: Shoulder Pivot
    robotParts.j2 = new THREE.Group();
    robotParts.j2.position.y = 5.8;
    robotParts.j2.rotation.y = THREE.MathUtils.degToRad(90);
    robotParts.j1.add(robotParts.j2);

    // J3: Elbow Pivot
    robotParts.j3 = new THREE.Group();
    robotParts.j3.position.y = L1 - 0.25;
    robotParts.j3.position.x = -2.5;
    robotParts.j2.add(robotParts.j3);

    // J4: Wrist Pitch Pivot
    robotParts.j4 = new THREE.Group();
    robotParts.j4.position.y = L2;
    robotParts.j3.add(robotParts.j4);

    // J5: Wrist Roll Pivot (Same physical location as Pitch, just different rotation axis)
    robotParts.j5 = new THREE.Group();
    robotParts.j5.position.y = 7.06;
    robotParts.j4.add(robotParts.j5);

    // J6: Gripper Pivot
    robotParts.j6 = new THREE.Group();
    robotParts.j6.position.y = L3;
    robotParts.j5.add(robotParts.j6);

    robotParts.gripperJaw = new THREE.Group();
    robotParts.gripperJaw.position.y = -14.5;
    robotParts.gripperJaw.position.z = 2.5;
    robotParts.j6.add(robotParts.gripperJaw);

    // We optionally add small AxesHelpers to each joint so you can see the invisible pivots.
    // Red=X, Green=Y, Blue=Z. Comment these out once your models look right.
    scene.add(new THREE.AxesHelper(5)); // World Origin
    robotParts.j2.add(new THREE.AxesHelper(3));
    robotParts.j3.add(new THREE.AxesHelper(3));
}

/**
 * Step 2: Load the GLB models and attach them to the skeleton
 */
function loadGLTFModels() {
    const loader = new GLTFLoader();

    // 1. Load the Base (Static, attached to World/Scene)
    loader.load('./models/base.glb', (gltf) => {
        const mesh = gltf.scene;
        // mesh.scale.set(10, 10, 10); // Adjust if your CAD exported in a different unit scale
        // mesh.rotation.x = -Math.PI / 2; // Sometimes CAD models import laying on their side. Use this to stand them up.

        // The base model should be added to j1 if it rotates, or the scene if it is completely static.
        // Assuming the 'base' file contains the static bottom AND the yawing part, attach to j1.
        robotParts.base.add(mesh);
    }, undefined, (error) => console.error('Error loading base:', error));

    loader.load('./models/rotating_base.glb', (gltf) => {
        const mesh = gltf.scene;
        robotParts.j1.add(mesh);
    });

    // 2. Load the Shoulder (Attached to J2)
    loader.load('./models/shoulder.glb', (gltf) => {
        const mesh = gltf.scene;
        // mesh.position.y = -BASE_HEIGHT; // Example: Offset if the CAD origin isn't at the pivot
        robotParts.j2.add(mesh);
    });

    // 3. Load the Elbow (Attached to J3)
    loader.load('./models/elbow.glb', (gltf) => {
        const mesh = gltf.scene;
        mesh.rotation.y = THREE.MathUtils.degToRad(90); // Example: Rotate if the CAD model's forward isn't aligned with the joint axis
        robotParts.j3.add(mesh);
    });

    // 4. Load the Wrist + Gripper (Attached to J5 so it rolls)
    // If you plan to animate the gripper claws opening/closing later, 
    // you will eventually need to split this into separate files.
    loader.load('./models/Wrist Pitch.glb', (gltf) => {
        const mesh = gltf.scene;
        mesh.rotation.y = THREE.MathUtils.degToRad(-90); // Example: Rotate if needed to align with joint axes
        mesh.rotation.z = THREE.MathUtils.degToRad(-90); // Example: Rotate if needed to align with joint axes
        robotParts.j4.add(mesh);
    });

    loader.load('./models/Gripper fix.glb', (gltf) => {
        const mesh = gltf.scene;
        mesh.rotation.z = THREE.MathUtils.degToRad(90);
        mesh.rotation.x = THREE.MathUtils.degToRad(-90);
        // mesh.rotation.z = THREE.MathUtils.degToRad(-90);
        robotParts.j5.add(mesh);
    });

    loader.load('./models/Jaw.glb', (gltf) => {
        const mesh = gltf.scene;
        mesh.rotation.y = THREE.MathUtils.degToRad(-90);
        // mesh.rotation.x = THREE.MathUtils.degToRad(-90);
        robotParts.gripperJaw.add(mesh);
    });
}

/**
 * Maps physical servo angles to Three.js local coordinate radians.
 * This remains exactly the same as before!
 */
export function updateJointAngle(jointId, angleDeg) {
    if (!robotParts[jointId]) return;

    const rad = THREE.MathUtils.degToRad(angleDeg);

    switch (jointId) {
        case 'j1': // YAW (Rotates on Y)
            robotParts.j1.rotation.y = THREE.MathUtils.degToRad(90 + angleDeg);
            break;
        case 'j2': // SHOULDER (Rotates on Z)
            robotParts.j2.rotation.z = THREE.MathUtils.degToRad(90 - angleDeg);
            break;
        case 'j3': // ELBOW (Rotates on Z)
            if (angleDeg > 160) angleDeg = 160;
            else if (angleDeg < 0) angleDeg = 0;
            robotParts.j3.rotation.z = rad;
            break;
        case 'j4': // WRIST PITCH (Rotates on Z)
            robotParts.j4.rotation.z = -THREE.MathUtils.degToRad(angleDeg - 90);
            break;
        case 'j5': // WRIST ROLL (Rotates on Y)
            robotParts.j5.rotation.y = rad;
            break;
    }
}