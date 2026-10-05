## Ambiente de desenvolvimento

O desenvolvimento pode ser realizado diretamente no cluster Atlântica utilizando o **Visual Studio Code** e a extensão **Remote - SSH**.

---

## 1. Pré-requisitos

No computador local, é necessário possuir:

- Visual Studio Code;
- cliente OpenSSH;
- extensão **Remote - SSH** do VS Code;
- acesso SSH à máquina Sparta;
- acesso SSH ao cluster Atlântica.

Para verificar se o SSH está disponível:

```bash
ssh -V
```

---

## 2. Instalar a extensão Remote - SSH

No Visual Studio Code:

1. Abrir a aba **Extensions**;
2. Pesquisar por `Remote - SSH`;
3. Instalar a extensão da Microsoft.

---

## 3. Configurar o SSH

O arquivo de configuração SSH permite que o acesso ao Atlântica seja realizado automaticamente através da Sparta.

### Windows: notepad %USERPROFILE%\.ssh\config

O arquivo normalmente está localizado em:

```text
C:\Users\<usuario>\.ssh\config
```

### Linux/macOS

```text
~/.ssh/config
```

Adicionar as seguintes configurações:

```ssh
Host sparta
    HostName sparta.pucrs.br
    User <USUARIO-SPARTA>

Host atlantica
    HostName atlantica.lad.pucrs.br
    User pptm08
    ProxyJump sparta
```

Substituir o campo `<USUARIO-SPARTA>` pela parte inicial do e-mail da pucrs.

---

## 4. Testar a conexão

Antes de utilizar o VS Code, testar a conexão pelo terminal do computador local:

```bash
ssh atlantica
```

Se a configuração estiver correta, o SSH utilizará a Sparta como máquina intermediária e abrirá uma sessão diretamente no Atlântica.

---

## 5. Conectar o VS Code ao Atlântica

No CMD:
`code --remote ssh-remote+atlantica /home/pptm08/Trabalho/mpi`

Ou no Visual Studio Code:
1. Pressionar `Ctrl + Shift + P`;
2. Pesquisar por: Remote-SSH: Connect to Host
3. Selecionar: atlantica
4. Aguardar o estabelecimento da conexão.
5. Abrir novo terminal
---

## 6. Compilar o programa ()

O código-fonte principal está localizado em:

```text
vetor.c
```

A compilação de um programa MPI em C pode ser realizada utilizando `mpicc`:

```bash
mpicc vetor.c -o vetor
```

Isso irá gerar o executável:

```text
vetor
```

---

## 9. Executar testes MPI

Para testes iniciais, o programa pode ser executado com diferentes quantidades de processos.

Exemplo com 4 processos:

```bash
srun -N 2 -n 4 ./vetor
```

Nesse caso:

```text
Processo 0 → Mestre
Processo 1 → Escravo
Processo 2 → Escravo
Processo 3 → Escravo
```

---

## 10. Fluxo de desenvolvimento

Após a configuração inicial, o fluxo de trabalho pode ser realizado inteiramente pelo VS Code:

```text
Computador local
      │
      │ VS Code Remote SSH
      ▼
    Sparta
      │
      ▼
   Atlântica
      │
      ├── editar vetor.c
      ├── compilar com mpicc
      ├── executar testes
      ├── submeter jobs ao Slurm
      └── utilizar Git
```

Para salvar alterações no repositório:

```bash
git status
git add vetor.c
git commit -m "Descrição da alteração"
git push
```
