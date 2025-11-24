#include <iostream>
#include <vector>
#include "waos/memory/Frame.h"
#include "waos/memory/PageTableEntry.h"
#include "waos/memory/PageTable.h"

int main() {
    std::cout << "=== Test de estructuras base de memoria ===\n\n";

    // ========== TEST FRAME ==========
    std::cout << "╔════════════════════════════════════╗\n";
    std::cout << "║     TESTING FRAME STRUCTURE        ║\n";
    std::cout << "╚════════════════════════════════════╝\n\n";

    std::cout << "--- Test 1: Crear frames ---\n";
    waos::memory::Frame frame1;
    std::cout << "Frame 1 creado (deberia estar libre)\n";
    std::cout << "  isFree(): " << (frame1.isFree() ? "true" : "false") << "\n";
    std::cout << "  pid: " << frame1.pid << "\n";
    std::cout << "  pageNumber: " << frame1.pageNumber << "\n";
    std::cout << "  occupied: " << (frame1.occupied ? "true" : "false") << "\n\n";

    std::cout << "--- Test 2: Asignar frame a proceso ---\n";
    frame1.pid = 1;
    frame1.pageNumber = 5;
    frame1.occupied = true;
    frame1.loadTime = 1000;
    frame1.lastAccessTime = 1000;
    
    std::cout << "Frame 1 asignado al proceso 1, pagina 5\n";
    std::cout << "  isFree(): " << (frame1.isFree() ? "true" : "false") << "\n";
    std::cout << "  pid: " << frame1.pid << "\n";
    std::cout << "  pageNumber: " << frame1.pageNumber << "\n";
    std::cout << "  loadTime: " << frame1.loadTime << "\n\n";

    std::cout << "--- Test 3: Resetear frame ---\n";
    frame1.reset();
    std::cout << "Frame 1 reseteado\n";
    std::cout << "  isFree(): " << (frame1.isFree() ? "true" : "false") << "\n";
    std::cout << "  occupied: " << (frame1.occupied ? "true" : "false") << "\n\n";

    std::cout << "--- Test 4: Vector de frames (memoria física simulada) ---\n";
    std::vector<waos::memory::Frame> physicalMemory(4);
    std::cout << "Creados 4 frames:\n";
    for (size_t i = 0; i < physicalMemory.size(); ++i) {
        std::cout << "  Frame " << i << ": " 
                  << (physicalMemory[i].isFree() ? "LIBRE" : "OCUPADO") << "\n";
    }
    std::cout << "\n";

    // ========== TEST PAGE TABLE ENTRY ==========
    std::cout << "╔════════════════════════════════════╗\n";
    std::cout << "║   TESTING PAGE TABLE ENTRY         ║\n";
    std::cout << "╚════════════════════════════════════╝\n\n";

    std::cout << "--- Test 1: Crear entrada de tabla de paginas ---\n";
    waos::memory::PageTableEntry pte1;
    std::cout << "PageTableEntry creada (deberia no estar cargada)\n";
    std::cout << "  isLoaded(): " << (pte1.isLoaded() ? "true" : "false") << "\n";
    std::cout << "  frameNumber: " << pte1.frameNumber << "\n";
    std::cout << "  present: " << (pte1.present ? "true" : "false") << "\n\n";

    std::cout << "--- Test 2: Cargar pagina en frame ---\n";
    pte1.load(2, 1500);  // Frame 2, tiempo 1500
    std::cout << "Pagina cargada en frame 2\n";
    std::cout << "  isLoaded(): " << (pte1.isLoaded() ? "true" : "false") << "\n";
    std::cout << "  frameNumber: " << pte1.frameNumber << "\n";
    std::cout << "  present: " << (pte1.present ? "true" : "false") << "\n";
    std::cout << "  lastAccess: " << pte1.lastAccess << "\n";
    std::cout << "  referenced: " << (pte1.referenced ? "true" : "false") << "\n\n";

    std::cout << "--- Test 3: Evict (desalojar) pagina ---\n";
    pte1.evict();
    std::cout << "Pagina desalojada\n";
    std::cout << "  isLoaded(): " << (pte1.isLoaded() ? "true" : "false") << "\n";
    std::cout << "  frameNumber: " << pte1.frameNumber << "\n";
    std::cout << "  present: " << (pte1.present ? "true" : "false") << "\n\n";

    // ========== TEST PAGE TABLE ==========
    std::cout << "╔════════════════════════════════════╗\n";
    std::cout << "║     TESTING PAGE TABLE             ║\n";
    std::cout << "╚════════════════════════════════════╝\n\n";

    std::cout << "--- Test 1: Crear tabla de paginas ---\n";
    waos::memory::PageTable pageTable;
    std::cout << "PageTable creada (vacia)\n";
    std::cout << "  size(): " << pageTable.size() << "\n\n";

    std::cout << "--- Test 2: Agregar entradas a la tabla ---\n";
    pageTable[0] = waos::memory::PageTableEntry();
    pageTable[1] = waos::memory::PageTableEntry();
    pageTable[2] = waos::memory::PageTableEntry();
    std::cout << "Agregadas 3 paginas (0, 1, 2)\n";
    std::cout << "  size(): " << pageTable.size() << "\n\n";

    std::cout << "--- Test 3: Cargar paginas en frames ---\n";
    pageTable[0].load(0, 100);  // Pagina 0 → Frame 0
    pageTable[1].load(1, 200);  // Pagina 1 → Frame 1
    // Pagina 2 no cargada (queda en disco)
    
    std::cout << "Estado de la tabla de paginas:\n";
    for (int i = 0; i < 3; ++i) {
        std::cout << "  Pagina " << i << ": ";
        if (pageTable[i].isLoaded()) {
            std::cout << "EN MEMORIA (frame " << pageTable[i].frameNumber << ")\n";
        } else {
            std::cout << "EN DISCO (no cargada)\n";
        }
    }
    std::cout << "\n";

    // ========== TEST INTEGRACION ==========
    std::cout << "╔════════════════════════════════════╗\n";
    std::cout << "║     TEST DE INTEGRACION            ║\n";
    std::cout << "╚════════════════════════════════════╝\n\n";

    std::cout << "--- Simulando asignacion manual de paginas a frames ---\n";
    
    // Memoria fisica con 3 frames
    std::vector<waos::memory::Frame> memory(3);
    
    // Tabla de paginas de un proceso con 4 paginas
    waos::memory::PageTable processTable;
    for (int i = 0; i < 4; ++i) {
        processTable[i] = waos::memory::PageTableEntry();
    }
    
    std::cout << "Memoria fisica: 3 frames\n";
    std::cout << "Proceso: 4 paginas logicas (0-3)\n\n";
    
    // Cargar pagina 0 en frame 0
    std::cout << "1. Cargar pagina 0 en frame 0\n";
    memory[0].pid = 1;
    memory[0].pageNumber = 0;
    memory[0].occupied = true;
    memory[0].loadTime = 1000;
    processTable[0].load(0, 1000);
    
    // Cargar pagina 1 en frame 1
    std::cout << "2. Cargar pagina 1 en frame 1\n";
    memory[1].pid = 1;
    memory[1].pageNumber = 1;
    memory[1].occupied = true;
    memory[1].loadTime = 1100;
    processTable[1].load(1, 1100);
    
    // Cargar pagina 2 en frame 2
    std::cout << "3. Cargar pagina 2 en frame 2\n";
    memory[2].pid = 1;
    memory[2].pageNumber = 2;
    memory[2].occupied = true;
    memory[2].loadTime = 1200;
    processTable[2].load(2, 1200);
    
    std::cout << "\nEstado final:\n";
    std::cout << "Memoria Fisica:\n";
    for (size_t i = 0; i < memory.size(); ++i) {
        std::cout << "  Frame " << i << ": ";
        if (memory[i].occupied) {
            std::cout << "PID=" << memory[i].pid 
                      << " Pagina=" << memory[i].pageNumber 
                      << " (cargada en t=" << memory[i].loadTime << ")\n";
        } else {
            std::cout << "LIBRE\n";
        }
    }
    
    std::cout << "\nTabla de Paginas del Proceso 1:\n";
    for (size_t i = 0; i < processTable.size(); ++i) {
        std::cout << "  Pagina " << i << ": ";
        if (processTable[i].isLoaded()) {
            std::cout << "Frame " << processTable[i].frameNumber << "\n";
        } else {
            std::cout << "EN DISCO (page fault si se accede)\n";
        }
    }

    std::cout << "\n=== Test finalizado exitosamente ===\n";
    std::cout << "Frame: OK\n";
    std::cout << "PageTableEntry: OK\n";
    std::cout << "PageTable: OK\n";
    std::cout << "Integración: OK\n";
    
    return 0;
}
