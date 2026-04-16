import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';

// Extracted from your ESP32 Kinematics
const BASE_HEIGHT = 11.1; 
const L1 = 10.50;         
const L2 = 12.9;          
const L3 = 16.25;         

const robotParts = {};

export function initRobotScene(container) {
    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0x121212);

    // Backed the camera up to accommodate the ~50cm total reach
    const camera = new THREE.PerspectiveCamera(45, container.clientWidth / container.clientHeight, 0.1, 500);
    camera.position.set(40, 30, 40); 

    const renderer = new THREE.WebGLRenderer({ antialias: true });
    renderer.setSize(container.clientWidth, container.clientHeight);
    renderer.shadowMap.enabled = true;
    container.appendChild(renderer.domElement);

    const controls = new OrbitControls(camera, renderer.domElement);
    controls.target.set(0, 20, 0); // Point camera at the center of the arm
    controls.update();

    scene.add(new THREE.AmbientLight(0xffffff, 0.4));
    const dirLight = new THREE.DirectionalLight(0xffffff, 1);
    dirLight.position.set(20, 40, 20);
    dirLight.castShadow = true;
    scene.add(dirLight);
    
    // Expanded GridHelper for the larger scale
    scene.add(new THREE.GridHelper(100, 100, 0x444444, 0x222222));

    buildRobot(scene);

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

function buildRobot(scene) {
    const matBase = new THREE.MeshStandardMaterial({ color: 0x333333 });
    const matJoint = new THREE.MeshStandardMaterial({ color: 0xbb86fc });
    const matLink = new THREE.MeshStandardMaterial({ color: 0xcccccc });

    // --- BASE & YAW (J1) ---
    // The base rotates on the Y axis. We place J1 at Y=0.
    robotParts.j1 = new THREE.Group();
    scene.add(robotParts.j1);
    
    // The visual base cylinder. Height = BASE_HEIGHT. 
    // Shifted up by BASE_HEIGHT/2 so its bottom rests on the grid.
    const baseMesh = new THREE.Mesh(new THREE.CylinderGeometry(2.5, 3, BASE_HEIGHT, 32), matBase);
    baseMesh.position.y = BASE_HEIGHT / 2;
    robotParts.j1.add(baseMesh);

    // --- SHOULDER (J2) ---
    // Sits at the top of the base. Rotates on Z axis.
    robotParts.j2 = new THREE.Group();
    robotParts.j2.position.y = BASE_HEIGHT;
    robotParts.j1.add(robotParts.j2);
    
    const joint2 = new THREE.Mesh(new THREE.CylinderGeometry(1.5, 1.5, 2, 32).rotateX(Math.PI/2), matJoint);
    robotParts.j2.add(joint2);
    
    // Link 1 (L1 = 10.50). Shifted up by L1/2 so it connects J2 to J3.
    const link1 = new THREE.Mesh(new THREE.BoxGeometry(1.2, L1, 1.2), matLink);
    link1.position.y = L1 / 2;
    robotParts.j2.add(link1);

    // --- ELBOW (J3) ---
    // Sits at the end of L1. Rotates on Z axis.
    robotParts.j3 = new THREE.Group();
    robotParts.j3.position.y = L1;
    robotParts.j2.add(robotParts.j3);
    
    const joint3 = new THREE.Mesh(new THREE.CylinderGeometry(1.2, 1.2, 1.8, 32).rotateX(Math.PI/2), matJoint);
    robotParts.j3.add(joint3);
    
    // Link 2 (L2 = 12.9). Shifted up by L2/2.
    const link2 = new THREE.Mesh(new THREE.BoxGeometry(1, L2, 1), matLink);
    link2.position.y = L2 / 2;
    robotParts.j3.add(link2);

    // --- WRIST PITCH (J4) ---
    // Sits at the end of L2. Rotates on Z axis.
    robotParts.j4 = new THREE.Group();
    robotParts.j4.position.y = L2;
    robotParts.j3.add(robotParts.j4);
    
    const joint4 = new THREE.Mesh(new THREE.CylinderGeometry(1, 1, 1.5, 32).rotateX(Math.PI/2), matJoint);
    robotParts.j4.add(joint4);

    // --- WRIST ROLL (J5) ---
    // Sits at the same physical pivot point as Pitch, but rotates on the Y axis.
    robotParts.j5 = new THREE.Group();
    robotParts.j4.add(robotParts.j5);
    
    const joint5 = new THREE.Mesh(new THREE.CylinderGeometry(0.8, 0.8, 1.2, 32), matJoint);
    robotParts.j5.add(joint5);

    // Link 3 (L3 = 16.25). This represents the distance from the wrist to the gripper tip.
    const link3 = new THREE.Mesh(new THREE.BoxGeometry(0.6, L3, 0.6), matLink);
    link3.position.y = L3 / 2;
    robotParts.j5.add(link3);

    // --- GRIPPER (J6) ---
    // Sits at the very end of L3.
    robotParts.j6 = new THREE.Group();
    robotParts.j6.position.y = L3;
    robotParts.j5.add(robotParts.j6);
    
    const gripper = new THREE.Mesh(new THREE.BoxGeometry(2, 0.5, 1), matJoint);
    robotParts.j6.add(gripper);
}

/**
 * Maps physical servo angles to Three.js local coordinate radians.
 * @param {string} jointId - The ID of the joint ('j1', 'j2', etc.)
 * @param {number} angleDeg - The physical angle from the UI slider
 */
export function updateJointAngle(jointId, angleDeg) {
    if (!robotParts[jointId]) return;

    // Convert degrees to radians for Three.js
    const rad = THREE.MathUtils.degToRad(angleDeg);

    switch (jointId) {
        case 'j1': // YAW (Rotates on Y)
            // User Rule: 0 is Left, increases to Right. 90 is center.
            // Three.js: Positive Y rotation turns left.
            // Mapping: 90 deg = 0 rad. 0 deg = +90 rad (left). 180 deg = -90 rad (right).
            robotParts.j1.rotation.y = THREE.MathUtils.degToRad(90 - angleDeg);
            break;

        case 'j2': // SHOULDER (Rotates on Z)
            // User Rule: 0 is horizontal left. 90 is straight up. Increases to right.
            // Three.js: 0 rad is straight up (+Y). Positive Z rotation folds to the left.
            // Mapping: 90 deg = 0 rad. 0 deg = +90 rad (horizontal left).
            robotParts.j2.rotation.z = THREE.MathUtils.degToRad(90 - angleDeg);
            break;

        case 'j3': // ELBOW (Rotates on Z)
            // User Rule: 0 is inline with shoulder. Increases going down towards ground.
            // Three.js: 0 rad is inline. Negative Z rotation bends it "down" (forward).
            // Mapping: We apply negative radians to make it bend forward as angle increases.
            robotParts.j3.rotation.z = -rad;
            break;

        case 'j4': // WRIST PITCH (Rotates on Z)
            // User Rule: 90 is inline with elbow. 0 is downward towards arm.
            // Three.js: 0 rad is inline. Negative Z rotation bends it downward.
            // Mapping: 90 deg = 0 rad. 0 deg = -90 rad.
            robotParts.j4.rotation.z = THREE.MathUtils.degToRad(angleDeg - 90);
            break;

        case 'j5': // WRIST ROLL (Rotates on Y)
            // User Rule: Continuous rotation.
            // Three.js: Direct mapping.
            robotParts.j5.rotation.y = rad;
            break;
    }
}